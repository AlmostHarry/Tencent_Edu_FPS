#include "Variant_Shooter/EduShooterGameState.h"

#include "Net/UnrealNetwork.h"

void AEduShooterGameState::SetTeamScore(EEduTeam Team, int32 NewScore)
{
	if (!HasAuthority())
	{
		return;
	}

	if (Team == EEduTeam::Red)
	{
		MatchState.RedScore = NewScore;
	}
	else if (Team == EEduTeam::Blue)
	{
		MatchState.BlueScore = NewScore;
	}
	else
	{
		return;
	}

	ForceNetUpdate();
	BroadcastMatchState();
}

void AEduShooterGameState::SetMatchEnded(EEduTeam WinningTeam)
{
	if (!HasAuthority() || MatchState.bMatchEnded)
	{
		return;
	}

	MatchState.WinningTeam = WinningTeam;
	MatchState.bMatchEnded = true;
	ForceNetUpdate();
	BroadcastMatchState();
}

bool AEduShooterGameState::SetMatchMode(EEduMatchMode NewMode)
{
	if (!HasAuthority() || MatchState.MatchMode != EEduMatchMode::Unselected
		|| NewMode == EEduMatchMode::Unselected)
	{
		return false;
	}

	MatchState.MatchMode = NewMode;
	ForceNetUpdate();
	BroadcastMatchState();
	return true;
}

void AEduShooterGameState::SetMatchStarted()
{
	if (!HasAuthority() || MatchState.bMatchStarted)
	{
		return;
	}

	MatchState.bMatchStarted = true;
	ForceNetUpdate();
	BroadcastMatchState();
}

int32 AEduShooterGameState::GetTeamScore(EEduTeam Team) const
{
	if (Team == EEduTeam::Red)
	{
		return MatchState.RedScore;
	}
	if (Team == EEduTeam::Blue)
	{
		return MatchState.BlueScore;
	}
	return 0;
}

void AEduShooterGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AEduShooterGameState, MatchState);
}

void AEduShooterGameState::OnRep_MatchState()
{
	BroadcastMatchState();
}

void AEduShooterGameState::BroadcastMatchState()
{
	OnMatchSetupChanged.Broadcast(MatchState.MatchMode, MatchState.bMatchStarted);
	OnTeamScoreChanged.Broadcast(static_cast<uint8>(EEduTeam::Red), MatchState.RedScore);
	OnTeamScoreChanged.Broadcast(static_cast<uint8>(EEduTeam::Blue), MatchState.BlueScore);

	if (MatchState.bMatchEnded)
	{
		OnMatchEnded.Broadcast(MatchState.WinningTeam);
	}
}
