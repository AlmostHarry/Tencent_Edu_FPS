// Copyright Epic Games, Inc. All Rights Reserved.


#include "Variant_Shooter/ShooterPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "Engine/LocalPlayer.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/PlayerStart.h"
#include "ShooterCharacter.h"
#include "ShooterGameMode.h"
#include "ShooterBulletCounterUI.h"
#include "EduTeamSelectionWidget.h"
#include "Tencent_Edu_FPS.h"
#include "Widgets/Input/SVirtualJoystick.h"

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

		// Until multiplayer slot ownership exists, only the first local player may select a slot.
		if (UGameplayStatics::GetPlayerController(GetWorld(), 0) == this)
		{
			ShowTeamSelection();
		}
	}
}

void AShooterPlayerController::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	if (AShooterGameMode* GameMode = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		GameMode->ReleasePlayerSlot(this);
	}

	Super::EndPlay(EndPlayReason);
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

	// subscribe to the pawn's OnDestroyed delegate
	InPawn->OnDestroyed.AddDynamic(this, &AShooterPlayerController::OnPawnDestroyed);

	// is this a shooter character?
	if (AShooterCharacter* ShooterCharacter = Cast<AShooterCharacter>(InPawn))
	{
		// add the player tag
		ShooterCharacter->Tags.Add(PlayerPawnTag);

		// subscribe to the pawn's delegates
		ShooterCharacter->OnBulletCountUpdated.AddDynamic(this, &AShooterPlayerController::OnBulletCountUpdated);
		ShooterCharacter->OnDamaged.AddDynamic(this, &AShooterPlayerController::OnPawnDamaged);

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
	TeamSelectionWidget = CreateWidget<UEduTeamSelectionWidget>(this, UEduTeamSelectionWidget::StaticClass());
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

bool AShooterPlayerController::SelectTeamSlot(EEduTeam Team, int32 SlotIndex)
{
	if (!IsLocalPlayerController() || UGameplayStatics::GetPlayerController(GetWorld(), 0) != this)
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

	if (AShooterGameMode* GameMode = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (!GameMode->ClaimPlayerSlot(this, RequestedSelection))
		{
			return false;
		}
	}
	else
	{
		return false;
	}

	TeamSlotSelection = RequestedSelection;
	bHasSelectedTeamSlot = true;

	const TCHAR* TeamName = Team == EEduTeam::Red ? TEXT("Red") : TEXT("Blue");
	UE_LOG(LogTencent_Edu_FPS, Log, TEXT("Local player selected team %s slot %d."), TeamName, SlotIndex);

	if (TeamSelectionWidget)
	{
		TeamSelectionWidget->RemoveFromParent();
		TeamSelectionWidget = nullptr;
	}

	FInputModeGameOnly InputMode;
	SetInputMode(InputMode);
	bShowMouseCursor = false;

	return true;
}
