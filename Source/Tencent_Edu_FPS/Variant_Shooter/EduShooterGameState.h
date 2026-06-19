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

USTRUCT(BlueprintType)
struct FEduScoreboardEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Shooter|Scoreboard")
	FEduTeamSlotSelection Selection;

	UPROPERTY(BlueprintReadOnly, Category="Shooter|Scoreboard")
	FString DisplayName;

	UPROPERTY(BlueprintReadOnly, Category="Shooter|Scoreboard")
	bool bHuman = false;

	UPROPERTY(BlueprintReadOnly, Category="Shooter|Scoreboard")
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly, Category="Shooter|Scoreboard")
	int32 Deaths = 0;

	UPROPERTY(BlueprintReadOnly, Category="Shooter|Scoreboard")
	int32 Assists = 0;
};

DECLARE_MULTICAST_DELEGATE_TwoParams(FEduTeamScoreChangedDelegate, uint8, int32);
DECLARE_MULTICAST_DELEGATE_OneParam(FEduMatchEndedDelegate, EEduTeam);
DECLARE_MULTICAST_DELEGATE_TwoParams(FEduMatchSetupChangedDelegate, EEduMatchMode, bool);
DECLARE_MULTICAST_DELEGATE_OneParam(FEduScoreboardChangedDelegate, const TArray<FEduScoreboardEntry>&);

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
	FEduScoreboardChangedDelegate OnScoreboardChanged;

	void SetTeamScore(EEduTeam Team, int32 NewScore);
	void SetMatchEnded(EEduTeam WinningTeam);
	bool SetMatchMode(EEduMatchMode NewMode);
	void SetMatchStarted();
	void InitializeScoreboard(const TArray<FEduTeamSlotSelection>& Selections);
	void SetScoreboardOccupant(const FEduTeamSlotSelection& Selection, const FString& DisplayName, bool bHuman);
	void AddScoreboardKill(const FEduTeamSlotSelection& Selection);
	void AddScoreboardDeath(const FEduTeamSlotSelection& Selection);
	void AddScoreboardAssist(const FEduTeamSlotSelection& Selection);

	int32 GetTeamScore(EEduTeam Team) const;
	bool IsMatchEnded() const { return MatchState.bMatchEnded; }
	EEduTeam GetWinningTeam() const { return MatchState.WinningTeam; }
	EEduMatchMode GetMatchMode() const { return MatchState.MatchMode; }
	bool HasMatchStarted() const { return MatchState.bMatchStarted; }
	const TArray<FEduScoreboardEntry>& GetScoreboardEntries() const { return ScoreboardEntries; }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(ReplicatedUsing=OnRep_MatchState)
	FEduReplicatedMatchState MatchState;

	UPROPERTY(ReplicatedUsing=OnRep_ScoreboardEntries)
	TArray<FEduScoreboardEntry> ScoreboardEntries;

	UFUNCTION()
	void OnRep_MatchState();

	UFUNCTION()
	void OnRep_ScoreboardEntries();

	void BroadcastMatchState();
	void BroadcastScoreboard();
	FEduScoreboardEntry* FindScoreboardEntry(const FEduTeamSlotSelection& Selection);
};
