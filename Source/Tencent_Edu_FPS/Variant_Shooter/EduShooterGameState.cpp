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

void AEduShooterGameState::SetExpectedHumanPlayerCount(int32 NewPlayerCount)
{
	if (!HasAuthority() || MatchState.MatchMode != EEduMatchMode::Unselected
		|| MatchState.ExpectedHumanPlayerCount == NewPlayerCount)
	{
		return;
	}

	MatchState.ExpectedHumanPlayerCount = NewPlayerCount;
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

void AEduShooterGameState::InitializeScoreboard(const TArray<FEduTeamSlotSelection>& Selections)
{
	if (!HasAuthority())
	{
		return;
	}

	ScoreboardEntries.Reset();
	for (const FEduTeamSlotSelection& Selection : Selections)
	{
		if (!Selection.IsValid())
		{
			continue;
		}

		FEduScoreboardEntry& Entry = ScoreboardEntries.AddDefaulted_GetRef();
		Entry.Selection = Selection;
		Entry.DisplayName = TEXT("Empty");
	}

	ForceNetUpdate();
	BroadcastScoreboard();
}

void AEduShooterGameState::SetScoreboardOccupant(
	const FEduTeamSlotSelection& Selection,
	const FString& DisplayName,
	bool bHuman)
{
	if (!HasAuthority())
	{
		return;
	}

	FEduScoreboardEntry* Entry = FindScoreboardEntry(Selection);
	if (!Entry)
	{
		return;
	}

	Entry->DisplayName = DisplayName;
	Entry->bHuman = bHuman;
	ForceNetUpdate();
	BroadcastScoreboard();
}

void AEduShooterGameState::AddScoreboardKill(const FEduTeamSlotSelection& Selection)
{
	if (!HasAuthority())
	{
		return;
	}

	FEduScoreboardEntry* Entry = FindScoreboardEntry(Selection);
	if (!Entry)
	{
		return;
	}

	++Entry->Kills;
	ForceNetUpdate();
	BroadcastScoreboard();
}

void AEduShooterGameState::AddScoreboardDeath(const FEduTeamSlotSelection& Selection)
{
	if (!HasAuthority())
	{
		return;
	}

	FEduScoreboardEntry* Entry = FindScoreboardEntry(Selection);
	if (!Entry)
	{
		return;
	}

	++Entry->Deaths;
	ForceNetUpdate();
	BroadcastScoreboard();
}

void AEduShooterGameState::AddScoreboardAssist(const FEduTeamSlotSelection& Selection)
{
	if (!HasAuthority())
	{
		return;
	}

	FEduScoreboardEntry* Entry = FindScoreboardEntry(Selection);
	if (!Entry)
	{
		return;
	}

	++Entry->Assists;
	ForceNetUpdate();
	BroadcastScoreboard();
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
	DOREPLIFETIME(AEduShooterGameState, ScoreboardEntries);
}

void AEduShooterGameState::OnRep_MatchState()
{
	BroadcastMatchState();
}

void AEduShooterGameState::OnRep_ScoreboardEntries()
{
	BroadcastScoreboard();
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

void AEduShooterGameState::BroadcastScoreboard()
{
	OnScoreboardChanged.Broadcast(ScoreboardEntries);
}

FEduScoreboardEntry* AEduShooterGameState::FindScoreboardEntry(const FEduTeamSlotSelection& Selection)
{
	return ScoreboardEntries.FindByPredicate([&Selection](const FEduScoreboardEntry& Entry)
	{
		return Entry.Selection.Team == Selection.Team && Entry.Selection.SlotIndex == Selection.SlotIndex;
	});
}
