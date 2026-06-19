#include "Variant_Shooter/UI/EduKDAWidget.h"

void UEduKDAWidget::SetKDA(int32 Kills, int32 Deaths, int32 Assists)
{
	CachedKills = Kills;
	CachedDeaths = Deaths;
	CachedAssists = Assists;
	BroadcastKDAUpdated();
}

FText UEduKDAWidget::GetKDAText() const
{
	return FText::FromString(FString::Printf(
		TEXT("K/D/A %d/%d/%d"),
		CachedKills,
		CachedDeaths,
		CachedAssists));
}

void UEduKDAWidget::NativeConstruct()
{
	Super::NativeConstruct();
	BroadcastKDAUpdated();
}

void UEduKDAWidget::BroadcastKDAUpdated()
{
	BP_OnKDAUpdated(CachedKills, CachedDeaths, CachedAssists, GetKDAText());
}
