// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "EduTeamSlotTypes.h"
#include "GameFramework/PlayerController.h"
#include "ShooterPlayerController.generated.h"

class UInputMappingContext;
class AShooterCharacter;
class UShooterBulletCounterUI;
class UEduMatchResultWidget;
class UEduTeamSelectionWidget;

/**
 *  Simple PlayerController for a first person shooter game
 *  Manages input mappings
 *  Respawns the player pawn when it's destroyed
 */
UCLASS(abstract, config="Game")
class TENCENT_EDU_FPS_API AShooterPlayerController : public APlayerController
{
	GENERATED_BODY()
	
protected:

	/** Input mapping contexts for this player */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Input Mapping Contexts */
	UPROPERTY(EditAnywhere, Category="Input|Input Mappings")
	TArray<UInputMappingContext*> MobileExcludedMappingContexts;

	/** Mobile controls widget to spawn */
	UPROPERTY(EditAnywhere, Category="Input|Touch Controls")
	TSubclassOf<UUserWidget> MobileControlsWidgetClass;

	/** Pointer to the mobile controls widget */
	UPROPERTY()
	TObjectPtr<UUserWidget> MobileControlsWidget;

	/** If true, the player will use UMG touch controls even if not playing on mobile platforms */
	UPROPERTY(EditAnywhere, Config, Category = "Input|Touch Controls")
	bool bForceTouchControls = false;

	/** Character class to respawn when the possessed pawn is destroyed */
	UPROPERTY(EditAnywhere, Category="Shooter|Respawn")
	TSubclassOf<AShooterCharacter> CharacterClass;

	/** Type of bullet counter UI widget to spawn */
	UPROPERTY(EditAnywhere, Category="Shooter|UI")
	TSubclassOf<UShooterBulletCounterUI> BulletCounterUIClass;

	/** Type of match result UI widget to spawn */
	UPROPERTY(EditDefaultsOnly, Category="Shooter|UI")
	TSubclassOf<UEduMatchResultWidget> MatchResultWidgetClass;

	/** Tag to grant the possessed pawn to flag it as the player */
	UPROPERTY(EditAnywhere, Category="Shooter|Player")
	FName PlayerPawnTag = FName("Player");

	/** Pointer to the bullet counter UI widget */
	UPROPERTY()
	TObjectPtr<UShooterBulletCounterUI> BulletCounterUI;

	/** Temporary team-slot selection UI for the first local player */
	UPROPERTY()
	TObjectPtr<UEduTeamSelectionWidget> TeamSelectionWidget;

	/** Simple match result overlay */
	UPROPERTY()
	TObjectPtr<UEduMatchResultWidget> MatchResultWidget;

	/** Team and slot selected by this player */
	UPROPERTY(BlueprintReadOnly, Category="Shooter|Team", meta=(AllowPrivateAccess="true"))
	FEduTeamSlotSelection TeamSlotSelection;

	/** True after this player has selected a valid team slot */
	UPROPERTY(BlueprintReadOnly, Category="Shooter|Team", meta=(AllowPrivateAccess="true"))
	bool bHasSelectedTeamSlot = false;

protected:

	/** Gameplay Initialization */
	virtual void BeginPlay() override;

	/** Gameplay cleanup */
	virtual void EndPlay(EEndPlayReason::Type EndPlayReason) override;

	/** Initialize input bindings */
	virtual void SetupInputComponent() override;

	/** Pawn initialization */
	virtual void OnPossess(APawn* InPawn) override;

	/** Called if the possessed pawn is destroyed */
	UFUNCTION()
	void OnPawnDestroyed(AActor* DestroyedActor);

	/** Called when the bullet count on the possessed pawn is updated */
	UFUNCTION()
	void OnBulletCountUpdated(int32 MagazineSize, int32 Bullets);

	/** Called when the possessed pawn is damaged */
	UFUNCTION()
	void OnPawnDamaged(float LifePercent);

	/** Returns true if the player should use UMG touch controls */
	bool ShouldUseTouchControls() const;

	/** Shows the temporary team-slot selector */
	void ShowTeamSelection();

public:

	/** Records a local player's team-slot choice */
	UFUNCTION(BlueprintCallable, Category="Shooter|Team")
	bool SelectTeamSlot(EEduTeam Team, int32 SlotIndex);

	/** Shows WIN or LOSE based on the local player's selected team */
	void ShowMatchResult(EEduTeam WinningTeam);
};
