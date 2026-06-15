// Copyright Epic Games, Inc. All Rights Reserved.


#include "ShooterCharacter.h"
#include "ShooterWeapon.h"
#include "EnhancedInputComponent.h"
#include "Components/InputComponent.h"
#include "Components/PawnNoiseEmitterComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Engine/World.h"
#include "Camera/CameraComponent.h"
#include "TimerManager.h"
#include "ShooterGameMode.h"
#include "EduShooterGameState.h"
#include "Net/UnrealNetwork.h"

AShooterCharacter::AShooterCharacter()
{
	// create the noise emitter component
	PawnNoiseEmitter = CreateDefaultSubobject<UPawnNoiseEmitterComponent>(TEXT("Pawn Noise Emitter"));

	// configure movement
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 600.0f, 0.0f);
}

void AShooterCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		CurrentHP = MaxHP;
		bIsDead = false;
	}

	// update the HUD
	OnDamaged.Broadcast(1.0f);
}

void AShooterCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AShooterCharacter, CurrentHP);
	DOREPLIFETIME(AShooterCharacter, CurrentWeapon);
	DOREPLIFETIME(AShooterCharacter, bIsDead);
	DOREPLIFETIME(AShooterCharacter, RespawnEndServerTime);
}

void AShooterCharacter::EndPlay(EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	// clear the respawn timer
	GetWorld()->GetTimerManager().ClearTimer(RespawnTimer);
	GetWorld()->GetTimerManager().ClearTimer(LocalRefireTimer);
}

void AShooterCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// base class handles move, aim and jump inputs
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// Firing
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AShooterCharacter::DoStartFiring);
		EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AShooterCharacter::DoStopFiring);

		// Switch weapon
		EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Triggered, this, &AShooterCharacter::DoSwitchWeapon);
	}

}

float AShooterCharacter::TakeDamage(float Damage, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (!HasAuthority())
	{
		return 0.0f;
	}

	// ignore if already dead
	if (CurrentHP <= 0.0f)
	{
		return 0.0f;
	}

	if (const ATencent_Edu_FPSCharacter* Attacker = EventInstigator ? Cast<ATencent_Edu_FPSCharacter>(EventInstigator->GetPawn()) : nullptr)
	{
		if (GetTeam() != EEduTeam::Unassigned && GetTeam() == Attacker->GetTeam())
		{
			return 0.0f;
		}
	}

	// Reduce HP
	CurrentHP -= Damage;

	// Have we depleted HP?
	if (CurrentHP <= 0.0f)
	{
		Die(EventInstigator);
	}

	// update the HUD
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));

	return Damage;
}

void AShooterCharacter::DoAim(float Yaw, float Pitch)
{
	// only route inputs if the character is not dead
	if (CanProcessGameplayInput())
	{
		Super::DoAim(Yaw, Pitch);

		if (bLocallyWantsToFire && IsLocallyControlled())
		{
			const FVector AimTarget = CalculateLocalWeaponTargetLocation();
			if (HasAuthority())
			{
				SetServerWeaponTargetLocation(AimTarget);
			}
			else
			{
				ServerUpdateAimTarget(AimTarget);
			}
		}
	}
}

void AShooterCharacter::DoMove(float Right, float Forward)
{
	// only route inputs if the character is not dead
	if (CanProcessGameplayInput())
	{
		Super::DoMove(Right, Forward);
	}
}

void AShooterCharacter::DoJumpStart()
{
	// only route inputs if the character is not dead
	if (CanProcessGameplayInput())
	{
		Super::DoJumpStart();
	}
}

void AShooterCharacter::DoJumpEnd()
{
	// only route inputs if the character is not dead
	if (CanProcessGameplayInput())
	{
		Super::DoJumpEnd();
	}
}

void AShooterCharacter::DoStartFiring()
{
	if (!CanProcessGameplayInput() || bLocallyWantsToFire)
	{
		return;
	}

	bLocallyWantsToFire = true;
	RequestFireShot();

	if (CurrentWeapon && CurrentWeapon->IsFullAuto())
	{
		const float RefireRate = FMath::Max(CurrentWeapon->GetRefireRate(), KINDA_SMALL_NUMBER);
		GetWorld()->GetTimerManager().SetTimer(
			LocalRefireTimer,
			this,
			&AShooterCharacter::RequestFireShot,
			RefireRate,
			true,
			RefireRate);
	}
}

void AShooterCharacter::DoStopFiring()
{
	bLocallyWantsToFire = false;
	GetWorld()->GetTimerManager().ClearTimer(LocalRefireTimer);
}

void AShooterCharacter::DoSwitchWeapon()
{
	if (!CanProcessGameplayInput())
	{
		return;
	}

	if (HasAuthority())
	{
		ServerSwitchWeapon_Implementation();
	}
	else
	{
		ServerSwitchWeapon();
	}
}

void AShooterCharacter::RequestFireShot()
{
	if (!bLocallyWantsToFire || !CurrentWeapon || !CanProcessGameplayInput())
	{
		GetWorld()->GetTimerManager().ClearTimer(LocalRefireTimer);
		return;
	}

	const FVector AimTarget = CalculateLocalWeaponTargetLocation();
	if (HasAuthority())
	{
		ServerFireShot_Implementation(AimTarget);
	}
	else
	{
		ServerFireShot(AimTarget);
	}
}

void AShooterCharacter::ServerFireShot_Implementation(FVector_NetQuantize AimTarget)
{
	SetServerWeaponTargetLocation(AimTarget);

	if (CurrentWeapon && CanProcessGameplayInput())
	{
		CurrentWeapon->TryFireOnce();
	}
}

void AShooterCharacter::ServerUpdateAimTarget_Implementation(FVector_NetQuantize AimTarget)
{
	SetServerWeaponTargetLocation(AimTarget);
}

void AShooterCharacter::ServerSwitchWeapon_Implementation()
{
	// ensure we have at least two weapons two switch between
	if (OwnedWeapons.Num() > 1 && CanProcessGameplayInput())
	{
		// deactivate the old weapon
		CurrentWeapon->DeactivateWeapon();

		// find the index of the current weapon in the owned list
		int32 WeaponIndex = OwnedWeapons.Find(CurrentWeapon);

		// is this the last weapon?
		if (WeaponIndex == OwnedWeapons.Num() - 1)
		{
			// loop back to the beginning of the array
			WeaponIndex = 0;
		}
		else {
			// select the next weapon index
			++WeaponIndex;
		}

		// set the new weapon as current
		CurrentWeapon = OwnedWeapons[WeaponIndex];

		// activate the new weapon
		CurrentWeapon->ActivateWeapon();
		ForceNetUpdate();
	}
}

void AShooterCharacter::AttachWeaponMeshes(AShooterWeapon* Weapon)
{
	const FAttachmentTransformRules AttachmentRule(EAttachmentRule::SnapToTarget, false);

	// attach the weapon actor
	Weapon->AttachToActor(this, AttachmentRule);

	// attach the weapon meshes
	Weapon->GetFirstPersonMesh()->AttachToComponent(GetFirstPersonMesh(), AttachmentRule, FirstPersonWeaponSocket);
	Weapon->GetThirdPersonMesh()->AttachToComponent(GetMesh(), AttachmentRule, ThirdPersonWeaponSocket);
	
}

void AShooterCharacter::PlayFiringMontage(UAnimMontage* Montage)
{
	// stub
}

void AShooterCharacter::AddWeaponRecoil(float Recoil)
{
	// apply the recoil as pitch input
	AddControllerPitchInput(Recoil);
}

void AShooterCharacter::UpdateWeaponHUD(int32 CurrentAmmo, int32 MagazineSize)
{
	OnBulletCountUpdated.Broadcast(MagazineSize, CurrentAmmo);
}

FVector AShooterCharacter::GetWeaponTargetLocation()
{
	if (HasAuthority() && !IsLocallyControlled() && bHasServerWeaponTarget)
	{
		return ServerWeaponTargetLocation;
	}

	return CalculateLocalWeaponTargetLocation();
}

FVector AShooterCharacter::CalculateLocalWeaponTargetLocation() const
{
	// trace ahead from the camera viewpoint
	FHitResult OutHit;

	const FVector Start = GetFirstPersonCameraComponent()->GetComponentLocation();
	const FVector End = Start + (GetFirstPersonCameraComponent()->GetForwardVector() * MaxAimDistance);

	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(this);

	GetWorld()->LineTraceSingleByChannel(OutHit, Start, End, ECC_Visibility, QueryParams);

	// return either the impact point or the trace end
	return OutHit.bBlockingHit ? OutHit.ImpactPoint : OutHit.TraceEnd;
}

void AShooterCharacter::SetServerWeaponTargetLocation(const FVector& AimTarget)
{
	const FVector AimOrigin = GetPawnViewLocation();
	FVector AimOffset = AimTarget - AimOrigin;

	if (AimTarget.ContainsNaN() || AimOffset.IsNearlyZero())
	{
		const FRotator ViewRotation = Controller ? Controller->GetControlRotation() : GetActorRotation();
		AimOffset = ViewRotation.Vector() * MaxAimDistance;
	}

	ServerWeaponTargetLocation = AimOrigin + AimOffset.GetClampedToMaxSize(MaxAimDistance);
	bHasServerWeaponTarget = true;
}

bool AShooterCharacter::AddWeaponClass(const TSubclassOf<AShooterWeapon>& WeaponClass)
{
	if (!HasAuthority() || !WeaponClass)
	{
		return false;
	}

	// do we already own this weapon?
	AShooterWeapon* OwnedWeapon = FindWeaponOfType(WeaponClass);

	if (OwnedWeapon)
	{
		return false;
	}

	// spawn the new weapon
	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = this;
	SpawnParams.Instigator = this;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.TransformScaleMethod = ESpawnActorScaleMethod::MultiplyWithRoot;

	AShooterWeapon* AddedWeapon = GetWorld()->SpawnActor<AShooterWeapon>(WeaponClass, GetActorTransform(), SpawnParams);

	if (!AddedWeapon)
	{
		return false;
	}

	// add the weapon to the owned list
	OwnedWeapons.Add(AddedWeapon);

	// if we have an existing weapon, deactivate it
	if (CurrentWeapon)
	{
		CurrentWeapon->DeactivateWeapon();
	}

	// switch to the new weapon
	CurrentWeapon = AddedWeapon;
	CurrentWeapon->ActivateWeapon();
	ForceNetUpdate();
	return true;
}

void AShooterCharacter::OnWeaponActivated(AShooterWeapon* Weapon)
{
	// update the bullet counter
	OnBulletCountUpdated.Broadcast(Weapon->GetMagazineSize(), Weapon->GetBulletCount());

	// set the character mesh AnimInstances
	if (IsLocallyControlled())
	{
		GetFirstPersonMesh()->SetAnimInstanceClass(Weapon->GetFirstPersonAnimInstanceClass());
	}
	GetMesh()->SetAnimInstanceClass(Weapon->GetThirdPersonAnimInstanceClass());
	RefreshFirstPersonPresentation();
}

void AShooterCharacter::OnWeaponDeactivated(AShooterWeapon* Weapon)
{
	// unused
}

void AShooterCharacter::OnSemiWeaponRefire()
{
	// unused
}

AShooterWeapon* AShooterCharacter::FindWeaponOfType(TSubclassOf<AShooterWeapon> WeaponClass) const
{
	// check each owned weapon
	for (AShooterWeapon* Weapon : OwnedWeapons)
	{
		if (Weapon->IsA(WeaponClass))
		{
			return Weapon;
		}
	}

	// weapon not found
	return nullptr;

}

void AShooterCharacter::Die(AController* KillerController)
{
	if (!HasAuthority() || bIsDead)
	{
		return;
	}

	bIsDead = true;
	CurrentHP = 0.0f;
	RespawnEndServerTime = GetWorld()->GetTimeSeconds() + RespawnTime;

	HandleDeathVisuals();

	// increment the killer's team score
	if (AShooterGameMode* GM = Cast<AShooterGameMode>(GetWorld()->GetAuthGameMode()))
	{
		if (const ATencent_Edu_FPSCharacter* Killer = KillerController ? Cast<ATencent_Edu_FPSCharacter>(KillerController->GetPawn()) : nullptr)
		{
			GM->IncrementTeamScore(static_cast<uint8>(Killer->GetTeam()));
		}
	}

	// schedule character respawn
	GetWorld()->GetTimerManager().SetTimer(RespawnTimer, this, &AShooterCharacter::OnRespawn, RespawnTime, false);
	ForceNetUpdate();
}

void AShooterCharacter::OnRespawn()
{
	if (HasAuthority())
	{
		Destroy();
	}
}

bool AShooterCharacter::IsDead() const
{
	return bIsDead;
}

void AShooterCharacter::OnRep_CurrentHP()
{
	OnDamaged.Broadcast(FMath::Max(0.0f, CurrentHP / MaxHP));
}

void AShooterCharacter::OnRep_CurrentWeapon()
{
	if (CurrentWeapon)
	{
		OnWeaponActivated(CurrentWeapon);
	}
}

void AShooterCharacter::OnRep_IsDead()
{
	if (bIsDead)
	{
		HandleDeathVisuals();
	}
}

void AShooterCharacter::HandleDeathVisuals()
{
	if (IsValid(CurrentWeapon))
	{
		CurrentWeapon->DeactivateWeapon();
	}

	Tags.AddUnique(DeathTag);
	GetCharacterMovement()->StopMovementImmediately();
	DisableInput(nullptr);
	OnBulletCountUpdated.Broadcast(0, 0);
	OnDeathStarted.Broadcast(RespawnEndServerTime);
	BP_OnDeath();
}

bool AShooterCharacter::CanProcessGameplayInput() const
{
	if (bIsDead)
	{
		return false;
	}

	const AEduShooterGameState* ShooterGameState = GetWorld()->GetGameState<AEduShooterGameState>();
	return !ShooterGameState || (ShooterGameState->HasMatchStarted() && !ShooterGameState->IsMatchEnded());
}
