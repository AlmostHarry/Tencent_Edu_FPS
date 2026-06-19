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

void AEduShooterPlayerState::AddKill()
{
	if (HasAuthority())
	{
		++MatchStats.Kills;
		BroadcastMatchStatsChanged();
		ForceNetUpdate();
	}
}

void AEduShooterPlayerState::AddDeath()
{
	if (HasAuthority())
	{
		++MatchStats.Deaths;
		BroadcastMatchStatsChanged();
		ForceNetUpdate();
	}
}

void AEduShooterPlayerState::AddAssist()
{
	if (HasAuthority())
	{
		++MatchStats.Assists;
		BroadcastMatchStatsChanged();
		ForceNetUpdate();
	}
}

void AEduShooterPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEduShooterPlayerState, TeamSlotSelection);
	DOREPLIFETIME(AEduShooterPlayerState, MatchStats);
}

void AEduShooterPlayerState::OnRep_MatchStats()
{
	BroadcastMatchStatsChanged();
}

void AEduShooterPlayerState::BroadcastMatchStatsChanged()
{
	OnMatchStatsChanged.Broadcast(MatchStats);
}
