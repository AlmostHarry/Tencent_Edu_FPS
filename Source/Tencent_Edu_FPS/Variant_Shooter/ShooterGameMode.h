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

struct FEduManagedMatchSlot
{
	FEduTeamSlotSelection Selection;
	TWeakObjectPtr<APlayerStart> PlayerStart;
	TWeakObjectPtr<AShooterPlayerController> HumanController;
	TWeakObjectPtr<AShooterNPC> AIPawn;
	FTimerHandle AIRespawnTimer;
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

	/** Optional override for the AI class used to fill unoccupied slots */
	UPROPERTY(EditAnywhere, Category="Shooter|Slots")
	TSubclassOf<AShooterNPC> AICharacterClass;

	/** Delay before an AI slot is refilled after death */
	UPROPERTY(EditAnywhere, Category="Shooter|Slots", meta=(ClampMin=0.0, Units="s"))
	float AIRespawnDelay = 3.0f;

	/** Four managed 2v2 match slots */
	TArray<FEduManagedMatchSlot> MatchSlots;

	bool bMatchShuttingDown = false;

protected:

	/** Gameplay initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

public:

	/** Increases the score for the given team */
	void IncrementTeamScore(uint8 TeamByte);

	/** Claims a free slot and moves the player's pawn to its spawn point */
	bool ClaimPlayerSlot(AShooterPlayerController* PlayerController, const FEduTeamSlotSelection& Selection);

	/** Releases a player's occupied slot */
	void ReleasePlayerSlot(AShooterPlayerController* PlayerController);

	/** Returns the spawn transform and team assigned to a player */
	bool GetPlayerSpawnData(const AShooterPlayerController* PlayerController, FTransform& OutTransform, EEduTeam& OutTeam) const;

private:

	void InitializeMatchSlots();
	void FillUnoccupiedSlotsWithAI();
	void SpawnAIForSlot(int32 SlotArrayIndex);
	int32 FindSlotIndex(const FEduTeamSlotSelection& Selection) const;
	int32 FindPlayerSlotIndex(const AShooterPlayerController* PlayerController) const;

	UFUNCTION()
	void OnManagedAIDestroyed(AActor* DestroyedActor);
};
