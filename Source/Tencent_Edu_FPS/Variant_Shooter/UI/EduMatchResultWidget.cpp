#include "Variant_Shooter/UI/EduMatchResultWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "ShooterPlayerController.h"

void UEduMatchResultWidget::SetMatchWon(bool bWon)
{
	BP_SetMatchResult(bWon);
}

void UEduMatchResultWidget::SetPlayerKDA(const FEduPlayerMatchStats& Stats)
{
	CachedStats = Stats;
	BroadcastPlayerKDAUpdated();
}

void UEduMatchResultWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RestartButton->OnClicked.AddUniqueDynamic(this, &UEduMatchResultWidget::RestartMatch);
	BroadcastPlayerKDAUpdated();
}

void UEduMatchResultWidget::BP_SetMatchResult_Implementation(bool bWon)
{
	ResultText->SetText(FText::FromString(bWon ? TEXT("WIN") : TEXT("LOSE")));
	ResultText->SetColorAndOpacity(bWon
		? FSlateColor(FLinearColor(0.1f, 0.85f, 0.2f, 1.0f))
		: FSlateColor(FLinearColor(0.9f, 0.08f, 0.08f, 1.0f)));
}

void UEduMatchResultWidget::RestartMatch()
{
	if (AShooterPlayerController* PlayerController = Cast<AShooterPlayerController>(GetOwningPlayer()))
	{
		PlayerController->RequestRestartMatch();
	}
}

FText UEduMatchResultWidget::GetPlayerKDAText() const
{
	return FText::FromString(FString::Printf(
		TEXT("Your KDA: %d/%d/%d"),
		CachedStats.Kills,
		CachedStats.Deaths,
		CachedStats.Assists));
}

void UEduMatchResultWidget::BroadcastPlayerKDAUpdated()
{
	BP_OnPlayerKDAUpdated(
		CachedStats.Kills,
		CachedStats.Deaths,
		CachedStats.Assists,
		GetPlayerKDAText());
}
