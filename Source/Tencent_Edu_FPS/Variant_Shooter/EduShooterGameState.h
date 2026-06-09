#pragma once

#include "CoreMinimal.h"
#include "EduTeamSlotTypes.h"
#include "GameFramework/GameStateBase.h"
#include "EduShooterGameState.generated.h"

UENUM(BlueprintType)
enum class EEduMatchMode : uint8
{
	Unselected,
	SinglePlayer,
	TwoPlayer
};

USTRUCT()
struct FEduReplicatedMatchState
{
	GENERATED_BODY()

	UPROPERTY()
	int32 RedScore = 0;

	UPROPERTY()
	int32 BlueScore = 0;

	UPROPERTY()
	EEduTeam WinningTeam = EEduTeam::Unassigned;

	UPROPERTY()
	bool bMatchEnded = false;

	UPROPERTY()
	EEduMatchMode MatchMode = EEduMatchMode::Unselected;

	UPROPERTY()
	bool bMatchStarted = false;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FEduTeamScoreChangedDelegate, uint8, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FEduMatchEndedDelegate, EEduTeam);
DECLARE_MULTICAST_DELEGATE_TwoParams(FEduMatchSetupChangedDelegate, EEduMatchMode, bool);

/**
 * Replicated match-wide state visible to every connected player.
 */
UCLASS()
class TENCENT_EDU_FPS_API AEduShooterGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	FEduTeamScoreChangedDelegate OnTeamScoreChanged;
	FEduMatchEndedDelegate OnMatchEnded;
	FEduMatchSetupChangedDelegate OnMatchSetupChanged;

	void SetTeamScore(EEduTeam Team, int32 NewScore);
	void SetMatchEnded(EEduTeam WinningTeam);
	bool SetMatchMode(EEduMatchMode NewMode);
	void SetMatchStarted();

	int32 GetTeamScore(EEduTeam Team) const;
	bool IsMatchEnded() const { return MatchState.bMatchEnded; }
	EEduTeam GetWinningTeam() const { return MatchState.WinningTeam; }
	EEduMatchMode GetMatchMode() const { return MatchState.MatchMode; }
	bool HasMatchStarted() const { return MatchState.bMatchStarted; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FEduReplicatedMatchState MatchState;

	UFUNCTION()
	void OnRep_MatchState();

	void BroadcastMatchState();
};
