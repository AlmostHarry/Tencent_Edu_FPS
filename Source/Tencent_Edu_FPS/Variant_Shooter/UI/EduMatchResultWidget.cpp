#include "Variant_Shooter/UI/EduMatchResultWidget.h"

#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void UEduMatchResultWidget::SetMatchWon(bool bWon)
{
	bMatchWon = bWon;
	InvalidateLayoutAndVolatility();
}

TSharedRef<SWidget> UEduMatchResultWidget::RebuildWidget()
{
	const FLinearColor ResultColor = bMatchWon
		? FLinearColor(0.1f, 0.85f, 0.2f, 1.0f)
		: FLinearColor(0.9f, 0.08f, 0.08f, 1.0f);
	FSlateFontInfo ResultFont = FAppStyle::GetFontStyle("HeadingMedium");
	ResultFont.Size = 96;
	FSlateFontInfo RestartFont = FAppStyle::GetFontStyle("HeadingMedium");
	RestartFont.Size = 32;

	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("WhiteBrush"))
		.BorderBackgroundColor(FLinearColor(0.0f, 0.0f, 0.0f, 0.72f))
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SVerticalBox)

			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				[
					SNew(STextBlock)
						.Text(FText::FromString(bMatchWon ? TEXT("WIN") : TEXT("LOSE")))
						.ColorAndOpacity(ResultColor)
						.Font(ResultFont)
						.Justification(ETextJustify::Center)
				]

			+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 28.0f, 0.0f, 0.0f)
				[
					SNew(SButton)
						.ContentPadding(FMargin(36.0f, 12.0f))
						.OnClicked_UObject(this, &UEduMatchResultWidget::RestartMatch)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("RESTART")))
								.Font(RestartFont)
								.Justification(ETextJustify::Center)
						]
				]
		];
}

FReply UEduMatchResultWidget::RestartMatch()
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetPause(false);
	}

	const FString LevelName = UGameplayStatics::GetCurrentLevelName(this, true);
	if (!LevelName.IsEmpty())
	{
		UGameplayStatics::OpenLevel(this, FName(*LevelName));
	}

	return FReply::Handled();
}
