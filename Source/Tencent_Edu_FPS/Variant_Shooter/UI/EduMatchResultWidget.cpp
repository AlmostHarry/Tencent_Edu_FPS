#include "Variant_Shooter/UI/EduMatchResultWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "ShooterPlayerController.h"

void UEduMatchResultWidget::SetMatchWon(bool bWon)
{
	BP_SetMatchResult(bWon);
}

void UEduMatchResultWidget::NativeConstruct()
{
	Super::NativeConstruct();
	RestartButton->OnClicked.AddUniqueDynamic(this, &UEduMatchResultWidget::RestartMatch);
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
