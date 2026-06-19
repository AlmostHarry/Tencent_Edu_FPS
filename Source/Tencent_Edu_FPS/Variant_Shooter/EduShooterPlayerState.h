#pragma once

#include "CoreMinimal.h"
#include "EduTeamSlotTypes.h"
#include "GameFramework/PlayerState.h"
#include "EduShooterPlayerState.generated.h"

USTRUCT(BlueprintType)
struct FEduPlayerMatchStats
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Shooter|Stats")
	int32 Kills = 0;

	UPROPERTY(BlueprintReadOnly, Category="Shooter|Stats")
	int32 Deaths = 0;

	UPROPERTY(BlueprintReadOnly, Category="Shooter|Stats")
	int32 Assists = 0;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FEduPlayerMatchStatsChangedDelegate, const FEduPlayerMatchStats&);

/**
 * Replicated identity for a human player's selected team and match slot.
 */
UCLASS()
class TENCENT_EDU_FPS_API AEduShooterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	void SetTeamSlotSelection(const FEduTeamSlotSelection& NewSelection);
	void ClearTeamSlotSelection();
	void AddKill();
	void AddDeath();
	void AddAssist();

	const FEduTeamSlotSelection& GetTeamSlotSelection() const { return TeamSlotSelection; }
	bool HasSelectedTeamSlot() const { return TeamSlotSelection.IsValid(); }
	const FEduPlayerMatchStats& GetMatchStats() const { return MatchStats; }

	FEduPlayerMatchStatsChangedDelegate OnMatchStatsChanged;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
	FEduTeamSlotSelection TeamSlotSelection;

	UPROPERTY(ReplicatedUsing=OnRep_MatchStats)
	FEduPlayerMatchStats MatchStats;

	UFUNCTION()
	void OnRep_MatchStats();

	void BroadcastMatchStatsChanged();
};
