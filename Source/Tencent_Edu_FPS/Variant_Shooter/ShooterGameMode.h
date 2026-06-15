// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EduTeamSlotTypes.h"
#include "GameFramework/GameModeBase.h"
#include "ShooterGameMode.generated.h"

class UShooterUI;
class APlayerStart;
class AShooterCharacter;
class AShooterNPC;
class AShooterPlayerController;
enum class EEduMatchMode : uint8;

struct FEduManagedMatchSlot
{
	FEduTeamSlotSelection Selection;
	TWeakObjectPtr<APlayerStart> PlayerStart;
	TWeakObjectPtr<AShooterPlayerController> HumanController;
	TWeakObjectPtr<AShooterNPC> AIPawn;
};

/**
 *  Simple GameMode for a first person shooter game
 *  Manages game UI
 *  Keeps track of team scores
 */
UCLASS(abstract)
class TENCENT_EDU_FPS_API AShooterGameMode : public AGameModeBase
{
	GENERATED_BODY()
	
protected:

	/** Type of UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter")
	TSubclassOf<UShooterUI> ShooterUIClass;

	/** Pointer to the UI widget */
	TObjectPtr<UShooterUI> ShooterUI;

	/** Map of scores by team ID */
	TMap<uint8, int32> TeamScores;

	/** Score required for a team to win the match */
	UPROPERTY(EditDefaultsOnly, Category="Shooter|Match", meta=(ClampMin=1))
	int32 WinningScore = 10;

	/** Optional override for the AI class used to fill unoccupied slots */
	UPROPERTY(EditAnywhere, Category="Shooter|Slots")
	TSubclassOf<AShooterNPC> AICharacterClass;

	/** Four managed 2v2 match slots */
	TArray<FEduManagedMatchSlot> MatchSlots;

	bool bMatchShuttingDown = false;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:
	AShooterGameMode();

	/** Increases the score for the given team */
	void IncrementTeamScore(uint8 TeamByte);

	/** Claims a free slot and moves the player's pawn to its spawn point */
	bool ClaimPlayerSlot(AShooterPlayerController* PlayerController, const FEduTeamSlotSelection& Selection);

	/** Accepts the listen server host's initial single-player/two-player choice */
	bool SelectMatchMode(AShooterPlayerController* PlayerController, EEduMatchMode MatchMode);

	/** Releases a player's occupied slot */
	void ReleasePlayerSlot(AShooterPlayerController* PlayerController);

	/** Returns the spawn transform and team assigned to a player */
	bool GetPlayerSpawnData(const AShooterPlayerController* PlayerController, FTransform& OutTransform, EEduTeam& OutTeam) const;

	/** Returns true after either team has reached the winning score */
	bool IsMatchOver() const { return bMatchEnded; }

	/** Restarts the current match for every connected player */
	void RestartNetworkMatch();

private:

	void InitializeMatchSlots();
	void TryStartMatch();
	int32 GetRequiredHumanPlayerCount() const;
	int32 GetSelectedHumanPlayerCount() const;
	bool IsPlayerAllowedForCurrentMode(const AShooterPlayerController* PlayerController) const;
	void FillUnoccupiedSlotsWithAI();
	void SpawnAIForSlot(int32 SlotArrayIndex);
	void FinishMatch(EEduTeam WinningTeam);
	int32 FindSlotIndex(const FEduTeamSlotSelection& Selection) const;
	int32 FindPlayerSlotIndex(const AShooterPlayerController* PlayerController) const;

	UFUNCTION()
	void OnManagedAIDestroyed(AActor* DestroyedActor);

	bool bMatchPopulationStarted = false;
	bool bMatchEnded = false;
};
