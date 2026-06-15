// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterGameMode.h"
#include "ShooterUI.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterPlayerController.h"
#include "EduShooterGameState.h"
#include "EduShooterPlayerState.h"
#include "AI/ShooterNPC.h"
#include "AI/ShooterNPCSpawner.h"
#include "TimerManager.h"

AShooterGameMode::AShooterGameMode()
{
	GameStateClass = AEduShooterGameState::StaticClass();
	PlayerStateClass = AEduShooterPlayerState::StaticClass();
}

void AShooterGameMode::BeginPlay()
{
	Super::BeginPlay();

	if (!GetGameState<AEduShooterGameState>())
	{
		UE_LOG(LogTemp, Error, TEXT("BP_ShooterGameMode must use EduShooterGameState for multiplayer."));
	}

	InitializeMatchSlots();

}

void AShooterGameMode::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	bMatchShuttingDown = true;

	Super::EndPlay(EndPlayReason);
}

void AShooterGameMode::IncrementTeamScore(uint8 TeamByte)
{
	if (bMatchEnded)
	{
		return;
	}

	const EEduTeam ScoringTeam = static_cast<EEduTeam>(TeamByte);
	if (ScoringTeam != EEduTeam::Red && ScoringTeam != EEduTeam::Blue)
	{
		return;
	}

	AEduShooterGameState* ShooterGameState = GetGameState<AEduShooterGameState>();
	if (!ShooterGameState)
	{
		return;
	}

	const int32 Score = ShooterGameState->GetTeamScore(ScoringTeam) + 1;
	ShooterGameState->SetTeamScore(ScoringTeam, Score);

	if (Score >= WinningScore)
	{
		FinishMatch(ScoringTeam);
	}
}

bool AShooterGameMode::ClaimPlayerSlot(AShooterPlayerController* PlayerController, const FEduTeamSlotSelection& Selection)
{
	if (!IsValid(PlayerController) || !Selection.IsValid() || bMatchPopulationStarted
		|| !IsPlayerAllowedForCurrentMode(PlayerController))
	{
		return false;
	}

	const int32 TargetSlotIndex = FindSlotIndex(Selection);
	if (!MatchSlots.IsValidIndex(TargetSlotIndex))
	{
		return false;
	}

	FEduManagedMatchSlot& TargetSlot = MatchSlots[TargetSlotIndex];
	if (TargetSlot.HumanController.IsValid())
	{
		return TargetSlot.HumanController.Get() == PlayerController;
	}

	const int32 PreviousSlotIndex = FindPlayerSlotIndex(PlayerController);
	if (PreviousSlotIndex == TargetSlotIndex)
	{
		return true;
	}

	if (MatchSlots.IsValidIndex(PreviousSlotIndex))
	{
		MatchSlots[PreviousSlotIndex].HumanController.Reset();
	}

	if (AShooterNPC* ReplacedAI = TargetSlot.AIPawn.Get())
	{
		TargetSlot.AIPawn.Reset();
		ReplacedAI->Destroy();
	}

	TargetSlot.HumanController = PlayerController;
	if (AEduShooterPlayerState* ShooterPlayerState = PlayerController->GetPlayerState<AEduShooterPlayerState>())
	{
		ShooterPlayerState->SetTeamSlotSelection(Selection);
	}

	if (AShooterCharacter* Character = Cast<AShooterCharacter>(PlayerController->GetPawn()))
	{
		PlayerController->SetPawnWaitingForMatch(false);
		Character->SetTeam(Selection.Team);

		if (TargetSlot.PlayerStart.IsValid())
		{
			const FTransform SpawnTransform = TargetSlot.PlayerStart->GetActorTransform();
			Character->TeleportTo(SpawnTransform.GetLocation(), SpawnTransform.Rotator());
			PlayerController->SetControlRotation(SpawnTransform.Rotator());
		}
	}

	TryStartMatch();
	return true;
}

bool AShooterGameMode::SelectMatchMode(AShooterPlayerController* PlayerController, EEduMatchMode MatchMode)
{
	if (!IsValid(PlayerController) || !PlayerController->IsLocalController()
		|| (MatchMode != EEduMatchMode::SinglePlayer && MatchMode != EEduMatchMode::TwoPlayer))
	{
		return false;
	}

	AEduShooterGameState* ShooterGameState = GetGameState<AEduShooterGameState>();
	if (!ShooterGameState || !ShooterGameState->SetMatchMode(MatchMode))
	{
		return false;
	}

	UE_LOG(LogTemp, Log, TEXT("Listen server selected %s mode."),
		MatchMode == EEduMatchMode::SinglePlayer ? TEXT("single-player") : TEXT("two-player"));
	return true;
}

void AShooterGameMode::ReleasePlayerSlot(AShooterPlayerController* PlayerController)
{
	const UWorld* World = GetWorld();
	if (bMatchShuttingDown || !World || World->bIsTearingDown)
	{
		return;
	}

	const int32 SlotIndex = FindPlayerSlotIndex(PlayerController);
	if (MatchSlots.IsValidIndex(SlotIndex))
	{
		MatchSlots[SlotIndex].HumanController.Reset();
		if (bMatchPopulationStarted)
		{
			FillUnoccupiedSlotsWithAI();
		}
	}

	if (IsValid(PlayerController))
	{
		if (AEduShooterPlayerState* ShooterPlayerState = PlayerController->GetPlayerState<AEduShooterPlayerState>())
		{
			ShooterPlayerState->ClearTeamSlotSelection();
		}
	}
}

bool AShooterGameMode::GetPlayerSpawnData(const AShooterPlayerController* PlayerController, FTransform& OutTransform, EEduTeam& OutTeam) const
{
	const int32 SlotIndex = FindPlayerSlotIndex(PlayerController);
	if (!MatchSlots.IsValidIndex(SlotIndex) || !MatchSlots[SlotIndex].PlayerStart.IsValid())
	{
		return false;
	}

	OutTransform = MatchSlots[SlotIndex].PlayerStart->GetActorTransform();
	OutTeam = MatchSlots[SlotIndex].Selection.Team;
	return true;
}

void AShooterGameMode::TryStartMatch()
{
	if (bMatchPopulationStarted)
	{
		return;
	}

	const int32 RequiredHumanPlayers = GetRequiredHumanPlayerCount();
	if (RequiredHumanPlayers <= 0 || GetSelectedHumanPlayerCount() < RequiredHumanPlayers)
	{
		return;
	}

	bMatchPopulationStarted = true;
	if (AEduShooterGameState* ShooterGameState = GetGameState<AEduShooterGameState>())
	{
		ShooterGameState->SetMatchStarted();
	}

	UE_LOG(LogTemp, Log, TEXT("%d required human player(s) selected slots; starting match."),
		RequiredHumanPlayers);
	FillUnoccupiedSlotsWithAI();
}

int32 AShooterGameMode::GetRequiredHumanPlayerCount() const
{
	const AEduShooterGameState* ShooterGameState = GetGameState<AEduShooterGameState>();
	if (!ShooterGameState)
	{
		return 0;
	}

	switch (ShooterGameState->GetMatchMode())
	{
	case EEduMatchMode::SinglePlayer:
		return 1;
	case EEduMatchMode::TwoPlayer:
		return 2;
	default:
		return 0;
	}
}

int32 AShooterGameMode::GetSelectedHumanPlayerCount() const
{
	int32 SelectedPlayerCount = 0;
	for (const FEduManagedMatchSlot& Slot : MatchSlots)
	{
		if (Slot.HumanController.IsValid())
		{
			++SelectedPlayerCount;
		}
	}
	return SelectedPlayerCount;
}

bool AShooterGameMode::IsPlayerAllowedForCurrentMode(const AShooterPlayerController* PlayerController) const
{
	const AEduShooterGameState* ShooterGameState = GetGameState<AEduShooterGameState>();
	if (!ShooterGameState)
	{
		return false;
	}

	if (ShooterGameState->GetMatchMode() == EEduMatchMode::SinglePlayer)
	{
		return PlayerController && PlayerController->IsLocalController();
	}

	return ShooterGameState->GetMatchMode() == EEduMatchMode::TwoPlayer;
}

void AShooterGameMode::InitializeMatchSlots()
{
	TArray<AActor*> SpawnerActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterNPCSpawner::StaticClass(), SpawnerActors);
	for (AActor* Actor : SpawnerActors)
	{
		if (AShooterNPCSpawner* Spawner = Cast<AShooterNPCSpawner>(Actor))
		{
			if (!AICharacterClass)
			{
				AICharacterClass = Spawner->GetNPCClass();
			}
			Spawner->DeactivateSpawner();
		}
	}

	TArray<AActor*> PlayerStartActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStartActors);

	TArray<APlayerStart*> RedStarts;
	TArray<APlayerStart*> BlueStarts;
	for (AActor* Actor : PlayerStartActors)
	{
		APlayerStart* PlayerStart = Cast<APlayerStart>(Actor);
		if (PlayerStart && PlayerStart->ActorHasTag(FName("RED")))
		{
			RedStarts.Add(PlayerStart);
		}
		else if (PlayerStart && PlayerStart->ActorHasTag(FName("BLUE")))
		{
			BlueStarts.Add(PlayerStart);
		}
	}

	auto SortStarts = [](const APlayerStart& Left, const APlayerStart& Right)
	{
		const FVector LeftLocation = Left.GetActorLocation();
		const FVector RightLocation = Right.GetActorLocation();
		if (!FMath::IsNearlyEqual(LeftLocation.X, RightLocation.X))
		{
			return LeftLocation.X < RightLocation.X;
		}
		if (!FMath::IsNearlyEqual(LeftLocation.Y, RightLocation.Y))
		{
			return LeftLocation.Y < RightLocation.Y;
		}
		return LeftLocation.Z < RightLocation.Z;
	};
	RedStarts.Sort(SortStarts);
	BlueStarts.Sort(SortStarts);

	MatchSlots.Reset();
	for (int32 SlotIndex = 0; SlotIndex < 2; ++SlotIndex)
	{
		if (RedStarts.IsValidIndex(SlotIndex))
		{
			FEduManagedMatchSlot& Slot = MatchSlots.AddDefaulted_GetRef();
			Slot.Selection.Team = EEduTeam::Red;
			Slot.Selection.SlotIndex = SlotIndex + 1;
			Slot.PlayerStart = RedStarts[SlotIndex];
		}
	}
	for (int32 SlotIndex = 0; SlotIndex < 2; ++SlotIndex)
	{
		if (BlueStarts.IsValidIndex(SlotIndex))
		{
			FEduManagedMatchSlot& Slot = MatchSlots.AddDefaulted_GetRef();
			Slot.Selection.Team = EEduTeam::Blue;
			Slot.Selection.SlotIndex = SlotIndex + 1;
			Slot.PlayerStart = BlueStarts[SlotIndex];
		}
	}

	UE_LOG(LogTemp, Log, TEXT("Initialized %d managed 2v2 slots using %d RED and %d BLUE PlayerStarts."),
		MatchSlots.Num(), RedStarts.Num(), BlueStarts.Num());

	if (MatchSlots.Num() != 4)
	{
		UE_LOG(LogTemp, Error, TEXT("The 2v2 match requires at least two RED and two BLUE tagged PlayerStarts."));
	}
	if (!AICharacterClass)
	{
		UE_LOG(LogTemp, Error, TEXT("No AI character class was found on the disabled Shooter NPC spawners."));
	}
}

void AShooterGameMode::FillUnoccupiedSlotsWithAI()
{
	const UWorld* World = GetWorld();
	if (bMatchShuttingDown || bMatchEnded || !bMatchPopulationStarted
		|| !World || World->bIsTearingDown)
	{
		return;
	}

	for (int32 SlotIndex = 0; SlotIndex < MatchSlots.Num(); ++SlotIndex)
	{
		FEduManagedMatchSlot& Slot = MatchSlots[SlotIndex];
		if (!Slot.HumanController.IsValid() && !Slot.AIPawn.IsValid())
		{
			SpawnAIForSlot(SlotIndex);
		}
	}
}

void AShooterGameMode::SpawnAIForSlot(int32 SlotArrayIndex)
{
	const UWorld* World = GetWorld();
	if (bMatchShuttingDown || bMatchEnded || !bMatchPopulationStarted
		|| !World || World->bIsTearingDown || !AICharacterClass
		|| !MatchSlots.IsValidIndex(SlotArrayIndex))
	{
		return;
	}

	FEduManagedMatchSlot& Slot = MatchSlots[SlotArrayIndex];
	if (!Slot.PlayerStart.IsValid() || Slot.HumanController.IsValid() || Slot.AIPawn.IsValid())
	{
		return;
	}

	const FTransform SpawnTransform = Slot.PlayerStart->GetActorTransform();
	AShooterNPC* NPC = GetWorld()->SpawnActorDeferred<AShooterNPC>(
		AICharacterClass,
		SpawnTransform,
		nullptr,
		nullptr,
		ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);

	if (NPC)
	{
		NPC->SetTeam(Slot.Selection.Team);
		NPC->OnDestroyed.AddDynamic(this, &AShooterGameMode::OnManagedAIDestroyed);
		Slot.AIPawn = NPC;
		NPC->FinishSpawning(SpawnTransform);

		const TCHAR* TeamName = Slot.Selection.Team == EEduTeam::Red ? TEXT("Red") : TEXT("Blue");
		UE_LOG(LogTemp, Log, TEXT("Spawned AI for %s slot %d."), TeamName, Slot.Selection.SlotIndex);
	}
}

void AShooterGameMode::FinishMatch(EEduTeam WinningTeam)
{
	if (bMatchEnded)
	{
		return;
	}

	bMatchEnded = true;

	const TCHAR* TeamName = WinningTeam == EEduTeam::Red ? TEXT("Red") : TEXT("Blue");
	UE_LOG(LogTemp, Log, TEXT("%s team won the match with %d points."), TeamName, WinningScore);

	if (AEduShooterGameState* ShooterGameState = GetGameState<AEduShooterGameState>())
	{
		ShooterGameState->SetMatchEnded(WinningTeam);
	}

	TArray<AActor*> NPCActors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AShooterNPC::StaticClass(), NPCActors);
	for (AActor* Actor : NPCActors)
	{
		if (AShooterNPC* NPC = Cast<AShooterNPC>(Actor))
		{
			NPC->StopShooting();
			if (AController* Controller = NPC->GetController())
			{
				Controller->StopMovement();
			}
		}
	}
}

void AShooterGameMode::RestartNetworkMatch()
{
	if (bMatchEnded)
	{
		GetWorld()->ServerTravel(TEXT("?restart"), false);
	}
}

int32 AShooterGameMode::FindSlotIndex(const FEduTeamSlotSelection& Selection) const
{
	return MatchSlots.IndexOfByPredicate([&Selection](const FEduManagedMatchSlot& Slot)
	{
		return Slot.Selection.Team == Selection.Team && Slot.Selection.SlotIndex == Selection.SlotIndex;
	});
}

int32 AShooterGameMode::FindPlayerSlotIndex(const AShooterPlayerController* PlayerController) const
{
	return MatchSlots.IndexOfByPredicate([PlayerController](const FEduManagedMatchSlot& Slot)
	{
		return Slot.HumanController.Get() == PlayerController;
	});
}

void AShooterGameMode::OnManagedAIDestroyed(AActor* DestroyedActor)
{
	const UWorld* World = GetWorld();
	if (bMatchShuttingDown || bMatchEnded || !World || World->bIsTearingDown)
	{
		return;
	}

	const int32 SlotIndex = MatchSlots.IndexOfByPredicate([DestroyedActor](const FEduManagedMatchSlot& Slot)
	{
		return Slot.AIPawn.GetEvenIfUnreachable() == DestroyedActor;
	});

	if (!MatchSlots.IsValidIndex(SlotIndex))
	{
		return;
	}

	FEduManagedMatchSlot& Slot = MatchSlots[SlotIndex];
	Slot.AIPawn.Reset();
	SpawnAIForSlot(SlotIndex);
}
