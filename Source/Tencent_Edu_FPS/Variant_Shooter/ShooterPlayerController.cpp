// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "UObject/ConstructorHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterGameMode.h"
#include "EduShooterGameState.h"
#include "EduShooterPlayerState.h"
#include "ShooterBulletCounterUI.h"
#include "ShooterUI.h"
#include "EduMatchResultWidget.h"
#include "EduMatchModeWidget.h"
#include "EduTeamSelectionWidget.h"
#include "Tencent_Edu_FPS.h"
#include "Widgets/Input/SVirtualJoystick.h"

AShooterPlayerController::AShooterPlayerController()
{
	static ConstructorHelpers::FClassFinder<UShooterUI> ShooterUIFinder(
		TEXT("/Game/Variant_Shooter/UI/UI_Shooter"));
	if (ShooterUIFinder.Succeeded())
	{
		ShooterUIClass = ShooterUIFinder.Class;
	}
}

void AShooterPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// only spawn touch controls on local player controllers
	if (IsLocalPlayerController())
	{
		if (ShouldUseTouchControls())
		{
			// spawn the mobile controls widget
			MobileControlsWidget = CreateWidget<UUserWidget>(this, MobileControlsWidgetClass);

			if (MobileControlsWidget)
			{
				// add the controls to the player screen
				MobileControlsWidget->AddToPlayerScreen(0);

			} else {

				UE_LOG(LogTencent_Edu_FPS, Error, TEXT("Could not spawn mobile controls widget."));

			}
		}

		// create the bullet counter widget and add it to the screen
		BulletCounterUI = CreateWidget<UShooterBulletCounterUI>(this, BulletCounterUIClass);

		if (BulletCounterUI)
		{
			BulletCounterUI->AddToPlayerScreen(0);

		} else {

			UE_LOG(LogTencent_Edu_FPS, Error, TEXT("Could not spawn bullet counter widget."));

		}

		ShooterUI = CreateWidget<UShooterUI>(this, ShooterUIClass);
		if (ShooterUI)
		{
			ShooterUI->AddToPlayerScreen(0);
		}

		BindToShooterGameState();
		InitializePossessedPawn(GetPawn());
	}
}

void AShooterPlayerController::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	if (World)
	{
		if (AEduShooterGameState* ShooterGameState = World->GetGameState<AEduShooterGameState>())
		{
			ShooterGameState->OnTeamScoreChanged.RemoveAll(this);
			ShooterGameState->OnMatchEnded.RemoveAll(this);
			ShooterGameState->OnMatchSetupChanged.RemoveAll(this);
		}
	}

	if (EndPlayReason != EEndPlayReason::EndPlayInEditor && World && !World->bIsTearingDown)
	{
		if (AShooterGameMode* GameMode = Cast<AShooterGameMode>(World->GetAuthGameMode()))
		{
			GameMode->ReleasePlayerSlot(this);
		}
	}

	Super::EndPlay(EndPlayReason);
}

void AShooterPlayerController::OnRep_Pawn()
{
	Super::OnRep_Pawn();
	InitializePossessedPawn(GetPawn());
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	// only add IMCs for local player controllers
	if (IsLocalPlayerController())
	{
		// add the input mapping contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}

			// only add these IMCs if we're not using mobile touch input
			if (!ShouldUseTouchControls())
			{
				for (UInputMappingContext* CurrentContext : MobileExcludedMappingContexts)
				{
					Subsystem->AddMappingContext(CurrentContext, 0);
				}
			}
		}
	}
}

void AShooterPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);
	InitializePossessedPawn(InPawn);

	if (HasAuthority())
	{
		const AEduShooterPlayerState* ShooterPlayerState = GetPlayerState<AEduShooterPlayerState>();
		SetPawnWaitingForMatch(!ShooterPlayerState || !ShooterPlayerState->HasSelectedTeamSlot());
	}
}

void AShooterPlayerController::InitializePossessedPawn(APawn* InPawn)
{
	if (!IsValid(InPawn))
	{
		return;
	}

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddUniqueDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

	// is this a shooter character?
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		// add the player tag
		ShooterCharacter->Tags.Add(PlayerPawnTag);

		// subscribe to the pawn's delegates
		ShooterCharacter->OnBulletCountUpdated.AddUniqueDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		ShooterCharacter->OnDamaged.AddUniqueDynamic(this, &AShooterPlayerController::OnPawnDamaged);

		// force update the life bar
		ShooterCharacter->OnDamaged.Broadcast(1.0f);
	}
}

void AShooterPlayerController::OnPawnDestroyed(AActor* DestroyedActor)
{
	// reset the bullet counter HUD
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_UpdateBulletCounter(0, 0);
	}

	if (!HasAuthority())
	{
		return;
	}

	UWorld* World = GetWorld();
	if (!World || World->bIsTearingDown)
	{
		return;
	}

	if (const AShooterGameMode* GameMode = Cast<AShooterGameMode>(World->GetAuthGameMode()))
	{
		if (GameMode->IsMatchOver())
		{
			return;
		}
	}

	FTransform SpawnTransform;
	EEduTeam RespawnTeam = EEduTeam::Unassigned;
	bool bHasManagedSpawn = false;
	if (AShooterGameMode* GameMode = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		bHasManagedSpawn = GameMode->GetPlayerSpawnData(this, SpawnTransform, RespawnTeam);
	}

	if (!bHasManagedSpawn)
	{
		TArray<AActor*> ActorList;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), ActorList);
		if (ActorList.Num() > 0)
		{
			SpawnTransform = ActorList[FMath::RandRange(0, ActorList.Num() - 1)]->GetActorTransform();
		}
		else
		{
			return;
		}
	}

	if (AShooterCharacter* RespawnedCharacter = GetWorld()->SpawnActor<AShooterCharacter>(CharacterClass, SpawnTransform))
	{
		RespawnedCharacter->SetTeam(RespawnTeam);
		Possess(RespawnedCharacter);
	}
}

void AShooterPlayerController::OnBulletCountUpdated(int32 MagazineSize, int32 Bullets)
{
	// update the UI
	if (BulletCounterUI)
	{
		BulletCounterUI->BP_UpdateBulletCounter(MagazineSize, Bullets);
	}
}

void AShooterPlayerController::OnPawnDamaged(float LifePercent)
{
	if (IsValid(BulletCounterUI))
	{
		BulletCounterUI->BP_Damaged(LifePercent);
	}
}

bool AShooterPlayerController::ShouldUseTouchControls() const
{
	// are we on a mobile platform? Should we force touch?
	return SVirtualJoystick::ShouldDisplayTouchInterface() || bForceTouchControls;
}

void AShooterPlayerController::ShowTeamSelection()
{
	if (TeamSelectionWidget)
	{
		return;
	}

	TeamSelectionWidget = CreateWidget<UEduTeamSelectionWidget>(this, TeamSelectionWidgetClass);
	if (!TeamSelectionWidget)
	{
		UE_LOG(LogTencent_Edu_FPS, Error, TEXT("Could not spawn team selection widget."));
		return;
	}

	TeamSelectionWidget->AddToPlayerScreen(100);

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AShooterPlayerController::ShowMatchModeScreen(EEduMatchModeWidgetState DisplayState)
{
	if (MatchModeWidget)
	{
		MatchModeWidget->RemoveFromParent();
		MatchModeWidget = nullptr;
	}

	MatchModeWidget = CreateWidget<UEduMatchModeWidget>(this, UEduMatchModeWidget::StaticClass());
	if (!MatchModeWidget)
	{
		UE_LOG(LogTencent_Edu_FPS, Error, TEXT("Could not spawn match mode widget."));
		return;
	}

	MatchModeWidget->SetDisplayState(DisplayState);
	MatchModeWidget->AddToPlayerScreen(110);

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = DisplayState == EEduMatchModeWidgetState::HostSelection;
}

void AShooterPlayerController::SelectMatchMode(EEduMatchMode MatchMode)
{
	if (!IsLocalPlayerController() || !HasAuthority())
	{
		return;
	}

	ServerSelectMatchMode_Implementation(MatchMode);
}

void AShooterPlayerController::ServerSelectMatchMode_Implementation(EEduMatchMode MatchMode)
{
	if (AShooterGameMode* GameMode = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->SelectMatchMode(this, MatchMode);
	}
}

bool AShooterPlayerController::SelectTeamSlot(EEduTeam Team, int32 SlotIndex)
{
	if (!IsLocalPlayerController())
	{
		return false;
	}

	FEduTeamSlotSelection RequestedSelection;
	RequestedSelection.Team = Team;
	RequestedSelection.SlotIndex = SlotIndex;
	if (!RequestedSelection.IsValid())
	{
		return false;
	}

	if (HasAuthority())
	{
		ServerSelectTeamSlot_Implementation(Team, SlotIndex);
	}
	else
	{
		ServerSelectTeamSlot(Team, SlotIndex);
	}

	return true;
}

void AShooterPlayerController::ServerSelectTeamSlot_Implementation(EEduTeam Team, int32 SlotIndex)
{
	FEduTeamSlotSelection RequestedSelection;
	RequestedSelection.Team = Team;
	RequestedSelection.SlotIndex = SlotIndex;

	bool bSuccess = false;
	if (RequestedSelection.IsValid())
	{
		if (AShooterGameMode* GameMode = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
		{
			bSuccess = GameMode->ClaimPlayerSlot(this, RequestedSelection);
		}
	}

	ClientTeamSlotSelectionResult(bSuccess, RequestedSelection);
}

void AShooterPlayerController::ClientTeamSlotSelectionResult_Implementation(
	bool bSuccess,
	FEduTeamSlotSelection Selection)
{
	if (bSuccess)
	{
		CompleteTeamSlotSelection(Selection);
	}
}

void AShooterPlayerController::CompleteTeamSlotSelection(const FEduTeamSlotSelection& Selection)
{
	if (!Selection.IsValid())
	{
		return;
	}

	TeamSlotSelection = Selection;
	bHasSelectedTeamSlot = true;

	const TCHAR* TeamName = Selection.Team == EEduTeam::Red ? TEXT("Red") : TEXT("Blue");
	UE_LOG(LogTencent_Edu_FPS, Log, TEXT("Local player selected team %s slot %d."),
		TeamName, Selection.SlotIndex);

	if (TeamSelectionWidget)
	{
		TeamSelectionWidget->RemoveFromParent();
		TeamSelectionWidget = nullptr;
	}

	const AEduShooterGameState* ShooterGameState = GetWorld()->GetGameState<AEduShooterGameState>();
	if (ShooterGameState && !ShooterGameState->HasMatchStarted())
	{
		ShowMatchModeScreen(EEduMatchModeWidgetState::WaitingForOtherPlayer);
	}
	else
	{
		FInputModeGameOnly InputMode;
		SetInputMode(InputMode);
		bShowMouseCursor = false;
	}
}

void AShooterPlayerController::ShowMatchResult(EEduTeam WinningTeam)
{
	if (!IsLocalPlayerController() || MatchResultWidget)
	{
		return;
	}

	if (TeamSelectionWidget)
	{
		TeamSelectionWidget->RemoveFromParent();
		TeamSelectionWidget = nullptr;
	}

	MatchResultWidget = CreateWidget<UEduMatchResultWidget>(this, MatchResultWidgetClass);
	if (!MatchResultWidget)
	{
		UE_LOG(LogTencent_Edu_FPS, Error, TEXT("Could not spawn match result widget."));
		return;
	}

	const bool bLocalPlayerWon = bHasSelectedTeamSlot && TeamSlotSelection.Team == WinningTeam;
	MatchResultWidget->SetMatchWon(bLocalPlayerWon);
	MatchResultWidget->AddToPlayerScreen(200);

	FInputModeUIOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = true;
}

void AShooterPlayerController::BindToShooterGameState()
{
	AEduShooterGameState* ShooterGameState = GetWorld()->GetGameState<AEduShooterGameState>();
	if (!ShooterGameState)
	{
		UE_LOG(LogTencent_Edu_FPS, Error, TEXT("Shooter GameState is not configured for multiplayer."));
		return;
	}

	ShooterGameState->OnTeamScoreChanged.AddUObject(this, &AShooterPlayerController::OnTeamScoreChanged);
	ShooterGameState->OnMatchEnded.AddUObject(this, &AShooterPlayerController::OnReplicatedMatchEnded);
	ShooterGameState->OnMatchSetupChanged.AddUObject(this, &AShooterPlayerController::OnMatchSetupChanged);

	OnMatchSetupChanged(ShooterGameState->GetMatchMode(), ShooterGameState->HasMatchStarted());
	OnTeamScoreChanged(
		static_cast<uint8>(EEduTeam::Red),
		ShooterGameState->GetTeamScore(EEduTeam::Red));
	OnTeamScoreChanged(
		static_cast<uint8>(EEduTeam::Blue),
		ShooterGameState->GetTeamScore(EEduTeam::Blue));

	if (ShooterGameState->IsMatchEnded())
	{
		OnReplicatedMatchEnded(ShooterGameState->GetWinningTeam());
	}
}

void AShooterPlayerController::OnTeamScoreChanged(uint8 TeamByte, int32 Score)
{
	if (ShooterUI)
	{
		ShooterUI->BP_UpdateScore(TeamByte, Score);
	}
}

void AShooterPlayerController::OnReplicatedMatchEnded(EEduTeam WinningTeam)
{
	ShowMatchResult(WinningTeam);
}

void AShooterPlayerController::OnMatchSetupChanged(EEduMatchMode MatchMode, bool bMatchStarted)
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (MatchMode == EEduMatchMode::Unselected)
	{
		ShowMatchModeScreen(HasAuthority()
			? EEduMatchModeWidgetState::HostSelection
			: EEduMatchModeWidgetState::WaitingForHost);
		return;
	}

	if (MatchMode == EEduMatchMode::SinglePlayer && !HasAuthority())
	{
		if (TeamSelectionWidget)
		{
			TeamSelectionWidget->RemoveFromParent();
			TeamSelectionWidget = nullptr;
		}
		ShowMatchModeScreen(EEduMatchModeWidgetState::SinglePlayerUnavailable);
		return;
	}

	if (MatchModeWidget)
	{
		MatchModeWidget->RemoveFromParent();
		MatchModeWidget = nullptr;
	}

	if (!bMatchStarted)
	{
		if (bHasSelectedTeamSlot)
		{
			ShowMatchModeScreen(EEduMatchModeWidgetState::WaitingForOtherPlayer);
		}
		else
		{
			ShowTeamSelection();
		}
		return;
	}

	if (TeamSelectionWidget)
	{
		TeamSelectionWidget->RemoveFromParent();
		TeamSelectionWidget = nullptr;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;
}

void AShooterPlayerController::RequestRestartMatch()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	if (HasAuthority())
	{
		ServerRestartMatch_Implementation();
	}
	else
	{
		ServerRestartMatch();
	}
}

void AShooterPlayerController::ServerRestartMatch_Implementation()
{
	if (AShooterGameMode* GameMode = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->RestartNetworkMatch();
	}
}

void AShooterPlayerController::SetPawnWaitingForMatch(bool bWaiting)
{
	if (!HasAuthority())
	{
		return;
	}

	AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(GetPawn());
	if (!ShooterCharacter)
	{
		return;
	}

	ShooterCharacter->SetActorHiddenInGame(bWaiting);
	ShooterCharacter->SetActorEnableCollision(!bWaiting);

	if (UCharacterMovementComponent* Movement = ShooterCharacter->GetCharacterMovement())
	{
		if (bWaiting)
		{
			Movement->DisableMovement();
		}
		else
		{
			Movement->SetMovementMode(MOVE_Walking);
		}
	}

	ShooterCharacter->ForceNetUpdate();
}
