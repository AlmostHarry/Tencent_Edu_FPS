#include "Variant_Shooter/UI/EduTeamSelectionWidget.h"

#include "EduTeamSlotTypes.h"
#include "ShooterPlayerController.h"
#include "Styling/AppStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Layout/SUniformGridPanel.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

TSharedRef<SWidget> UEduTeamSelectionWidget::RebuildWidget()
{
	return SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
		.Padding(24.0f)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBox)
			.WidthOverride(520.0f)
			[
				SNew(SVerticalBox)

				+ SVerticalBox::Slot()
				.AutoHeight()
				.HAlign(HAlign_Center)
				.Padding(0.0f, 0.0f, 0.0f, 18.0f)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("选择位置")))
					.Font(FAppStyle::GetFontStyle("HeadingMedium"))
				]

				+ SVerticalBox::Slot()
				.AutoHeight()
				[
					SNew(SUniformGridPanel)
					.SlotPadding(FMargin(8.0f))

					+ SUniformGridPanel::Slot(0, 0)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.ContentPadding(FMargin(28.0f, 16.0f))
						.OnClicked_UObject(this, &UEduTeamSelectionWidget::SelectRedSlotOne)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("红队 R1")))
						]
					]

					+ SUniformGridPanel::Slot(1, 0)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.ContentPadding(FMargin(28.0f, 16.0f))
						.OnClicked_UObject(this, &UEduTeamSelectionWidget::SelectRedSlotTwo)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("红队 R2")))
						]
					]

					+ SUniformGridPanel::Slot(0, 1)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.ContentPadding(FMargin(28.0f, 16.0f))
						.OnClicked_UObject(this, &UEduTeamSelectionWidget::SelectBlueSlotOne)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("蓝队 B1")))
						]
					]

					+ SUniformGridPanel::Slot(1, 1)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.ContentPadding(FMargin(28.0f, 16.0f))
						.OnClicked_UObject(this, &UEduTeamSelectionWidget::SelectBlueSlotTwo)
						[
							SNew(STextBlock)
								.Text(FText::FromString(TEXT("蓝队 B2")))
						]
					]
				]
			]
		];
}

FReply UEduTeamSelectionWidget::SelectRedSlotOne()
{
	return SelectSlot(static_cast<uint8>(EEduTeam::Red), 1);
}

FReply UEduTeamSelectionWidget::SelectRedSlotTwo()
{
	return SelectSlot(static_cast<uint8>(EEduTeam::Red), 2);
}

FReply UEduTeamSelectionWidget::SelectBlueSlotOne()
{
	return SelectSlot(static_cast<uint8>(EEduTeam::Blue), 1);
}

FReply UEduTeamSelectionWidget::SelectBlueSlotTwo()
{
	return SelectSlot(static_cast<uint8>(EEduTeam::Blue), 2);
}

FReply UEduTeamSelectionWidget::SelectSlot(uint8 TeamValue, int32 SlotIndex)
{
	if (AShooterPlayerController* Controller = Cast<AShooterPlayerController>(GetOwningPlayer()))
	{
		Controller->SelectTeamSlot(static_cast<EEduTeam>(TeamValue), SlotIndex);
	}

	return FReply::Handled();
}
