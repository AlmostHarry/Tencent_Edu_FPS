// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/EduTeamSlotTypes.h"
#include "GameFramework/Character.h"
#include "Logging/LogMacros.h"
#include "Tencent_Edu_FPSCharacter.generated.h"

class UInputComponent;
class USkeletalMeshComponent;
class UCameraComponent;
class UInputAction;
struct FInputActionValue;

DECLARE_LOG_CATEGORY_EXTERN(LogTemplateCharacter, Log, All);

/**
 *  A basic first person character
 */
UCLASS(abstract)
class ATencent_Edu_FPSCharacter : public ACharacter
{
	GENERATED_BODY()

	/** Pawn mesh: first person view (arms; seen only by self) */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	USkeletalMeshComponent* FirstPersonMesh;

	/** First person camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Components", meta = (AllowPrivateAccess = "true"))
	UCameraComponent* FirstPersonCameraComponent;

protected:

	/** Jump Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* JumpAction;

	/** Move Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	UInputAction* MoveAction;

	/** Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* LookAction;

	/** Mouse Look Input Action */
	UPROPERTY(EditAnywhere, Category ="Input")
	class UInputAction* MouseLookAction;

	/** Match team assigned by the slot system */
	UPROPERTY(ReplicatedUsing=OnRep_Team, VisibleInstanceOnly, BlueprintReadOnly, Category="Team")
	EEduTeam Team = EEduTeam::Unassigned;

	/** Body tint used for red team characters */
	UPROPERTY(EditDefaultsOnly, Category="Team|Visuals")
	FLinearColor RedTeamColor = FLinearColor(0.8f, 0.02f, 0.02f, 1.0f);

	/** Body tint used for blue team characters */
	UPROPERTY(EditDefaultsOnly, Category="Team|Visuals")
	FLinearColor BlueTeamColor = FLinearColor(0.02f, 0.08f, 0.8f, 1.0f);
	
public:
	ATencent_Edu_FPSCharacter();

protected:

	virtual void BeginPlay() override;

	/** Called from Input Actions for movement input */
	void MoveInput(const FInputActionValue& Value);

	/** Called from Input Actions for looking input */
	void LookInput(const FInputActionValue& Value);

	/** Handles aim inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoAim(float Yaw, float Pitch);

	/** Handles move inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoMove(float Right, float Forward);

	/** Handles jump start inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpStart();

	/** Handles jump end inputs from either controls or UI interfaces */
	UFUNCTION(BlueprintCallable, Category="Input")
	virtual void DoJumpEnd();

protected:

	/** Set up input action bindings */
	virtual void SetupPlayerInputComponent(UInputComponent* InputComponent) override;

	virtual void PossessedBy(AController* NewController) override;
	virtual void OnRep_Controller() override;
	virtual void PawnClientRestart() override;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	/** First-person animation must only run for the locally controlled pawn */
	void RefreshFirstPersonPresentation();
	

public:

	/** Returns the first person mesh **/
	USkeletalMeshComponent* GetFirstPersonMesh() const { return FirstPersonMesh; }

	/** Returns first person camera component **/
	UCameraComponent* GetFirstPersonCameraComponent() const { return FirstPersonCameraComponent; }

	/** Assigns this character to a match team */
	UFUNCTION(BlueprintCallable, Category="Team")
	void SetTeam(EEduTeam NewTeam);

	/** Returns this character's match team */
	UFUNCTION(BlueprintPure, Category="Team")
	EEduTeam GetTeam() const { return Team; }

	/** Returns true when both characters have assigned, different teams */
	UFUNCTION(BlueprintPure, Category="Team")
	bool IsEnemy(const ATencent_Edu_FPSCharacter* Other) const;

private:
	UFUNCTION()
	void OnRep_Team();

	/** Applies the selected team color to first- and third-person meshes */
	void ApplyTeamVisuals();

};

