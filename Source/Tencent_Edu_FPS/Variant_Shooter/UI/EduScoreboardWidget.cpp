#include "Variant_Shooter/UI/EduScoreboardWidget.h"

#include "Components/TextBlock.h"

namespace
{
FString GetScoreboardSlotLabel(const FEduTeamSlotSelection& Selection)
{
	const TCHAR* TeamPrefix = Selection.Team == EEduTeam::Red ? TEXT("R") : TEXT("B");
	return FString::Printf(TEXT("%s%d"), TeamPrefix, Selection.SlotIndex);
}

void AppendTeamEntries(FString& Output, const TArray<FEduScoreboardEntry>& Entries, EEduTeam Team)
{
	Output += Team == EEduTeam::Red ? TEXT("RED TEAM\n") : TEXT("BLUE TEAM\n");
	Output += TEXT("Slot  Name        K / D / A\n");

	for (const FEduScoreboardEntry& Entry : Entries)
	{
		if (Entry.Selection.Team != Team)
		{
			continue;
		}

		Output += FString::Printf(
			TEXT("%-4s  %-10s  %d / %d / %d\n"),
			*GetScoreboardSlotLabel(Entry.Selection),
			*Entry.DisplayName.Left(10),
			Entry.Kills,
			Entry.Deaths,
			Entry.Assists);
	}
}
}

void UEduScoreboardWidget::SetScoreboardEntries(const TArray<FEduScoreboardEntry>& Entries)
{
	CachedEntries = Entries;
	BroadcastScoreboardUpdated();
}

FText UEduScoreboardWidget::GetScoreboardText() const
{
	FString Output;
	AppendTeamEntries(Output, CachedEntries, EEduTeam::Red);
	Output += TEXT("\n");
	AppendTeamEntries(Output, CachedEntries, EEduTeam::Blue);
	return FText::FromString(Output);
}

void UEduScoreboardWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BroadcastScoreboardUpdated();
}

void UEduScoreboardWidget::BroadcastScoreboardUpdated()
{
	const FText DisplayText = GetScoreboardText();
	if (ScoreboardText)
	{
		ScoreboardText->SetText(DisplayText);
	}

	BP_OnScoreboardUpdated(CachedEntries, DisplayText);
}
