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
#include "EduRespawnCountdownWidget.h"
#include "EduScoreboardWidget.h"
#include "EduTeamSelectionWidget.h"
#include "EduKDAWidget.h"
#include "Tencent_Edu_FPS.h"
#include "InputCoreTypes.h"
#include "Widgets/Input/SVirtualJoystick.h"

AShooterPlayerController::AShooterPlayerController()
{
	static ConstructorHelpers::FClassFinder<UShooterUI> ShooterUIFinder(
		TEXT("/Game/Variant_Shooter/UI/UI_Shooter"));
	if (ShooterUIFinder.Succeeded())
	{
		ShooterUIClass = ShooterUIFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UEduMatchModeWidget> MatchModeWidgetFinder(
		TEXT("/Game/Variant_Shooter/UI/WBP_MatchMode"));
	if (MatchModeWidgetFinder.Succeeded())
	{
		MatchModeWidgetClass = MatchModeWidgetFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UEduRespawnCountdownWidget> RespawnWidgetFinder(
		TEXT("/Game/Variant_Shooter/UI/WBP_RespawnCountdown"));
	if (RespawnWidgetFinder.Succeeded())
	{
		RespawnCountdownWidgetClass = RespawnWidgetFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UEduKDAWidget> KDAWidgetFinder(
		TEXT("/Game/Variant_Shooter/UI/WBP_KDA"));
	if (KDAWidgetFinder.Succeeded())
	{
		KDAWidgetClass = KDAWidgetFinder.Class;
	}

	static ConstructorHelpers::FClassFinder<UEduScoreboardWidget> ScoreboardWidgetFinder(
		TEXT("/Game/Variant_Shooter/UI/WBP_Scoreboard"));
	if (ScoreboardWidgetFinder.Succeeded())
	{
		ScoreboardWidgetClass = ScoreboardWidgetFinder.Class;
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

		KDAWidget = CreateWidget<UEduKDAWidget>(this, KDAWidgetClass);
		if (KDAWidget)
		{
			KDAWidget->AddToPlayerScreen(50);
		}
		else
		{
			UE_LOG(LogTencent_Edu_FPS, Warning,
				TEXT("Could not spawn KDA widget. Create /Game/Variant_Shooter/UI/WBP_KDA as a child of EduKDAWidget."));
		}

		BindToShooterGameState();
		BindToShooterPlayerState();
		InitializePossessedPawn(GetPawn());
	}
}

void AShooterPlayerController::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	UWorld* World = GetWorld();
	HideRespawnCountdown();
	HideScoreboard();
	if (World)
	{
		if (AEduShooterGameState* ShooterGameState = World->GetGameState<AEduShooterGameState>())
		{
			ShooterGameState->OnTeamScoreChanged.RemoveAll(this);
			ShooterGameState->OnMatchEnded.RemoveAll(this);
			ShooterGameState->OnMatchSetupChanged.RemoveAll(this);
			ShooterGameState->OnScoreboardChanged.RemoveAll(this);
		}
		if (AEduShooterPlayerState* ShooterPlayerState = BoundStatsPlayerState.Get())
		{
			ShooterPlayerState->OnMatchStatsChanged.RemoveAll(this);
			BoundStatsPlayerState.Reset();
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

void AShooterPlayerController::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();
	BindToShooterPlayerState();
}

void AShooterPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (InputComponent)
	{
		InputComponent->BindKey(EKeys::Tab, IE_Pressed, this, &AShooterPlayerController::ShowScoreboard);
		InputComponent->BindKey(EKeys::Tab, IE_Released, this, &AShooterPlayerController::HideScoreboard);
	}

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

	if (IsLocalPlayerController())
	{
		HideRespawnCountdown();
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
		ShooterCharacter->OnDeathStarted.AddUniqueDynamic(this, &AShooterPlayerController::OnPawnDeathStarted);

		// force update the life bar
		ShooterCharacter->OnDamaged.Broadcast(1.0f);

		if (ShooterCharacter->IsDead())
		{
			OnPawnDeathStarted(ShooterCharacter->GetRespawnEndServerTime());
		}
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

	MatchModeWidget = CreateWidget<UEduMatchModeWidget>(this, MatchModeWidgetClass);
	if (!MatchModeWidget)
	{
		UE_LOG(LogTencent_Edu_FPS, Error, TEXT("Could not spawn match mode widget."));
		return;
	}

	const AEduShooterGameState* ShooterGameState = GetWorld()->GetGameState<AEduShooterGameState>();
	const int32 ExpectedHumanPlayerCount = ShooterGameState
		? ShooterGameState->GetExpectedHumanPlayerCount()
		: 0;
	MatchModeWidget->SetDisplayState(DisplayState, ExpectedHumanPlayerCount);
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

void AShooterPlayerController::OnPawnDeathStarted(float InRespawnEndServerTime)
{
	if (!IsLocalPlayerController() || InRespawnEndServerTime <= 0.0f)
	{
		return;
	}

	const AEduShooterGameState* ShooterGameState = GetWorld()->GetGameState<AEduShooterGameState>();
	if (ShooterGameState && ShooterGameState->IsMatchEnded())
	{
		return;
	}

	RespawnEndServerTime = InRespawnEndServerTime;
	if (!RespawnCountdownWidget)
	{
		RespawnCountdownWidget = CreateWidget<UEduRespawnCountdownWidget>(
			this,
			RespawnCountdownWidgetClass);
		if (!RespawnCountdownWidget)
		{
			UE_LOG(LogTencent_Edu_FPS, Error, TEXT("Could not spawn respawn countdown widget."));
			return;
		}

		RespawnCountdownWidget->AddToPlayerScreen(150);
	}

	UpdateRespawnCountdown();
	GetWorldTimerManager().SetTimer(
		RespawnCountdownTimer,
		this,
		&AShooterPlayerController::UpdateRespawnCountdown,
		0.1f,
		true);
}

void AShooterPlayerController::UpdateRespawnCountdown()
{
	if (!RespawnCountdownWidget)
	{
		GetWorldTimerManager().ClearTimer(RespawnCountdownTimer);
		return;
	}

	const AGameStateBase* GameState = GetWorld()->GetGameState();
	const float ServerTime = GameState
		? GameState->GetServerWorldTimeSeconds()
		: GetWorld()->GetTimeSeconds();
	const int32 RemainingSeconds = FMath::Max(
		0,
		FMath::CeilToInt(RespawnEndServerTime - ServerTime));
	RespawnCountdownWidget->SetRemainingSeconds(RemainingSeconds);

	if (RemainingSeconds <= 0)
	{
		GetWorldTimerManager().ClearTimer(RespawnCountdownTimer);
	}
}

void AShooterPlayerController::HideRespawnCountdown()
{
	GetWorldTimerManager().ClearTimer(RespawnCountdownTimer);
	RespawnEndServerTime = 0.0f;

	if (RespawnCountdownWidget)
	{
		RespawnCountdownWidget->RemoveFromParent();
		RespawnCountdownWidget = nullptr;
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

	HideRespawnCountdown();

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
	if (const AEduShooterPlayerState* ShooterPlayerState = GetPlayerState<AEduShooterPlayerState>())
	{
		MatchResultWidget->SetPlayerKDA(ShooterPlayerState->GetMatchStats());
	}
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
	ShooterGameState->OnScoreboardChanged.AddUObject(this, &AShooterPlayerController::OnScoreboardChanged);

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

	OnScoreboardChanged(ShooterGameState->GetScoreboardEntries());
}

void AShooterPlayerController::BindToShooterPlayerState()
{
	if (!IsLocalPlayerController())
	{
		return;
	}

	AEduShooterPlayerState* ShooterPlayerState = GetPlayerState<AEduShooterPlayerState>();
	if (BoundStatsPlayerState.Get() == ShooterPlayerState)
	{
		if (ShooterPlayerState)
		{
			OnPlayerMatchStatsChanged(ShooterPlayerState->GetMatchStats());
		}
		return;
	}

	if (AEduShooterPlayerState* PreviousPlayerState = BoundStatsPlayerState.Get())
	{
		PreviousPlayerState->OnMatchStatsChanged.RemoveAll(this);
	}

	BoundStatsPlayerState = ShooterPlayerState;
	if (ShooterPlayerState)
	{
		ShooterPlayerState->OnMatchStatsChanged.AddUObject(this, &AShooterPlayerController::OnPlayerMatchStatsChanged);
		OnPlayerMatchStatsChanged(ShooterPlayerState->GetMatchStats());
	}
}

void AShooterPlayerController::OnTeamScoreChanged(uint8 TeamByte, int32 Score)
{
	if (ShooterUI)
	{
		ShooterUI->BP_UpdateScore(TeamByte, Score);
	}
}

void AShooterPlayerController::OnScoreboardChanged(const TArray<FEduScoreboardEntry>& Entries)
{
	if (ScoreboardWidget)
	{
		ScoreboardWidget->SetScoreboardEntries(Entries);
	}
}

void AShooterPlayerController::OnPlayerMatchStatsChanged(const FEduPlayerMatchStats& Stats)
{
	if (KDAWidget)
	{
		KDAWidget->SetKDA(Stats.Kills, Stats.Deaths, Stats.Assists);
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

void AShooterPlayerController::ShowScoreboard()
{
	if (!IsLocalPlayerController() || ScoreboardWidget)
	{
		return;
	}

	if (!ScoreboardWidgetClass)
	{
		UE_LOG(LogTencent_Edu_FPS, Warning,
			TEXT("ScoreboardWidgetClass is not configured. Set it to WBP_Scoreboard in BP_ShooterPlayerController Class Defaults."));
		return;
	}

	ScoreboardWidget = CreateWidget<UEduScoreboardWidget>(this, ScoreboardWidgetClass);
	if (!ScoreboardWidget)
	{
		UE_LOG(LogTencent_Edu_FPS, Warning,
			TEXT("Could not spawn scoreboard widget. Create /Game/Variant_Shooter/UI/WBP_Scoreboard as a child of EduScoreboardWidget."));
		return;
	}

	if (const AEduShooterGameState* ShooterGameState = GetWorld()->GetGameState<AEduShooterGameState>())
	{
		ScoreboardWidget->SetScoreboardEntries(ShooterGameState->GetScoreboardEntries());
	}

	ScoreboardWidget->AddToPlayerScreen(175);
}

void AShooterPlayerController::HideScoreboard()
{
	if (ScoreboardWidget)
	{
		ScoreboardWidget->RemoveFromParent();
		ScoreboardWidget = nullptr;
	}
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
