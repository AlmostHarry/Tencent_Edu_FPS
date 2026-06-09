// Copyright Epic Games, Inc. All Rights Reserved.

#include "Tencent_Edu_FPSCharacter.h"
#include "Animation/AnimInstance.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "EnhancedInputComponent.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"
#include "Tencent_Edu_FPS.h"

ATencent_Edu_FPSCharacter::ATencent_Edu_FPSCharacter()
{
	bReplicates = true;
	SetReplicateMovement(true);

	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(55.f, 96.0f);
	
	// Create the first person mesh that will be viewed only by this character's owner
	FirstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("First Person Mesh"));

	FirstPersonMesh->SetupAttachment(GetMesh());
	FirstPersonMesh->SetOnlyOwnerSee(true);
	FirstPersonMesh->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::FirstPerson;
	FirstPersonMesh->SetCollisionProfileName(FName("NoCollision"));

	// Create the Camera Component	
	FirstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("First Person Camera"));
	FirstPersonCameraComponent->SetupAttachment(FirstPersonMesh, FName("head"));
	FirstPersonCameraComponent->SetRelativeLocationAndRotation(FVector(-2.8f, 5.89f, 0.0f), FRotator(0.0f, 90.0f, -90.0f));
	FirstPersonCameraComponent->bUsePawnControlRotation = true;
	FirstPersonCameraComponent->bEnableFirstPersonFieldOfView = true;
	FirstPersonCameraComponent->bEnableFirstPersonScale = true;
	FirstPersonCameraComponent->FirstPersonFieldOfView = 70.0f;
	FirstPersonCameraComponent->FirstPersonScale = 0.6f;

	// configure the character comps
	GetMesh()->SetOwnerNoSee(true);
	GetMesh()->FirstPersonPrimitiveType = EFirstPersonPrimitiveType::WorldSpaceRepresentation;

	GetCapsuleComponent()->SetCapsuleSize(34.0f, 96.0f);

	// Configure character movement
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;
	GetCharacterMovement()->AirControl = 0.5f;
}

void ATencent_Edu_FPSCharacter::BeginPlay()
{
	Super::BeginPlay();
	RefreshFirstPersonPresentation();
}

void ATencent_Edu_FPSCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	RefreshFirstPersonPresentation();
}

void ATencent_Edu_FPSCharacter::OnRep_Controller()
{
	Super::OnRep_Controller();
	RefreshFirstPersonPresentation();
}

void ATencent_Edu_FPSCharacter::PawnClientRestart()
{
	Super::PawnClientRestart();
	RefreshFirstPersonPresentation();
}

void ATencent_Edu_FPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{	
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ATencent_Edu_FPSCharacter::DoJumpStart);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ATencent_Edu_FPSCharacter::DoJumpEnd);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ATencent_Edu_FPSCharacter::MoveInput);

		// Looking/Aiming
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ATencent_Edu_FPSCharacter::LookInput);
		EnhancedInputComponent->BindAction(MouseLookAction, ETriggerEvent::Triggered, this, &ATencent_Edu_FPSCharacter::LookInput);
	}
	else
	{
		UE_LOG(LogTencent_Edu_FPS, Error, TEXT("'%s' Failed to find an Enhanced Input Component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}


void ATencent_Edu_FPSCharacter::MoveInput(const FInputActionValue& Value)
{
	// get the Vector2D move axis
	FVector2D MovementVector = Value.Get<FVector2D>();

	// pass the axis values to the move input
	DoMove(MovementVector.X, MovementVector.Y);

}

void ATencent_Edu_FPSCharacter::LookInput(const FInputActionValue& Value)
{
	// get the Vector2D look axis
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	// pass the axis values to the aim input
	DoAim(LookAxisVector.X, LookAxisVector.Y);

}

void ATencent_Edu_FPSCharacter::DoAim(float Yaw, float Pitch)
{
	if (GetController())
	{
		// pass the rotation inputs
		AddControllerYawInput(Yaw);
		AddControllerPitchInput(Pitch);
	}
}

void ATencent_Edu_FPSCharacter::DoMove(float Right, float Forward)
{
	if (GetController())
	{
		// pass the move inputs
		AddMovementInput(GetActorRightVector(), Right);
		AddMovementInput(GetActorForwardVector(), Forward);
	}
}

void ATencent_Edu_FPSCharacter::DoJumpStart()
{
	// pass Jump to the character
	Jump();
}

void ATencent_Edu_FPSCharacter::DoJumpEnd()
{
	// pass StopJumping to the character
	StopJumping();
}

void ATencent_Edu_FPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ATencent_Edu_FPSCharacter, Team);
}

void ATencent_Edu_FPSCharacter::SetTeam(EEduTeam NewTeam)
{
	if (!HasAuthority())
	{
		return;
	}

	Team = NewTeam;
	ApplyTeamVisuals();
	ForceNetUpdate();
}

void ATencent_Edu_FPSCharacter::OnRep_Team()
{
	ApplyTeamVisuals();
}

bool ATencent_Edu_FPSCharacter::IsEnemy(const ATencent_Edu_FPSCharacter* Other) const
{
	return IsValid(Other)
		&& Team != EEduTeam::Unassigned
		&& Other->Team != EEduTeam::Unassigned
		&& Team != Other->Team;
}

void ATencent_Edu_FPSCharacter::ApplyTeamVisuals()
{
	if (Team == EEduTeam::Unassigned)
	{
		return;
	}

	const FLinearColor TeamColor = Team == EEduTeam::Red ? RedTeamColor : BlueTeamColor;
	USkeletalMeshComponent* Meshes[] = { GetMesh(), FirstPersonMesh };

	for (USkeletalMeshComponent* MeshComponent : Meshes)
	{
		if (!MeshComponent)
		{
			continue;
		}

		const int32 MaterialCount = MeshComponent->GetNumMaterials();
		for (int32 MaterialIndex = 0; MaterialIndex < MaterialCount; ++MaterialIndex)
		{
			if (UMaterialInstanceDynamic* TeamMaterial = MeshComponent->CreateDynamicMaterialInstance(MaterialIndex))
			{
				TeamMaterial->SetVectorParameterValue(TEXT("Paint Tint"), TeamColor);
				TeamMaterial->SetVectorParameterValue(TEXT("Base Color"), TeamColor);
				TeamMaterial->SetVectorParameterValue(TEXT("LogoTint"), TeamColor);
			}
		}
	}
}

void ATencent_Edu_FPSCharacter::RefreshFirstPersonPresentation()
{
	if (!FirstPersonMesh)
	{
		return;
	}

	const bool bLocallyControlled = IsLocallyControlled();
	FirstPersonMesh->SetComponentTickEnabled(bLocallyControlled);
	FirstPersonMesh->SetVisibility(bLocallyControlled, true);
}
