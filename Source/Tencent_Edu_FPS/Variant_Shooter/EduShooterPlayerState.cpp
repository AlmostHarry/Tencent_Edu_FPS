#include "Variant_Shooter/EduShooterPlayerState.h"

#include "Net/UnrealNetwork.h"

void AEduShooterPlayerState::SetTeamSlotSelection(const FEduTeamSlotSelection& NewSelection)
{
	if (HasAuthority())
	{
		TeamSlotSelection = NewSelection;
		ForceNetUpdate();
	}
}

void AEduShooterPlayerState::ClearTeamSlotSelection()
{
	if (HasAuthority())
	{
		TeamSlotSelection = FEduTeamSlotSelection();
		ForceNetUpdate();
	}
}

void AEduShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEduShooterPlayerState, TeamSlotSelection);
}
