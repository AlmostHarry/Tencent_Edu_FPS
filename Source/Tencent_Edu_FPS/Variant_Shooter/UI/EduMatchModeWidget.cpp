#include "Variant_Shooter/UI/EduMatchModeWidget.h"

#include "EduShooterGameState.h"
#include "ShooterPlayerController.h"
#include "Styling/CoreStyle.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/SBoxPanel.h"
#include "Widgets/Text/STextBlock.h"

void UEduMatchModeWidget::SetDisplayState(EEduMatchModeWidgetState NewState)
{
	DisplayState = NewState;
}

TSharedRef<SWidget> UEduMatchModeWidget::RebuildWidget()
{
	FText Title = NSLOCTEXT("EduFPS", "WaitingForHost", "等待房主");
	if (DisplayState == EEduMatchModeWidgetState::HostSelection)
	{
		Title = NSLOCTEXT("EduFPS", "ChooseMatchMode", "选择游戏模式");
	}
	else if (DisplayState == EEduMatchModeWidgetState::SinglePlayerUnavailable)
	{
		Title = NSLOCTEXT("EduFPS", "SinglePlayerModeTitle", "单人模式");
	}
	else if (DisplayState == EEduMatchModeWidgetState::WaitingForOtherPlayer)
	{
		Title = NSLOCTEXT("EduFPS", "WaitingForOtherPlayer", "等待另一名玩家");
	}

	FText Description = NSLOCTEXT("EduFPS", "WaitingForHostDescription",
		"房主正在选择单人或双人模式。");
	if (DisplayState == EEduMatchModeWidgetState::HostSelection)
	{
		Description = NSLOCTEXT("EduFPS", "ChooseMatchModeDescription",
			"单人模式：1名玩家和3名AI。双人模式：等待2名玩家选位后加入2名AI。");
	}
	else if (DisplayState == EEduMatchModeWidgetState::SinglePlayerUnavailable)
	{
		Description = NSLOCTEXT("EduFPS", "SinglePlayerUnavailable",
			"房主选择了单人模式，当前客户端不参与本局游戏。");
	}
	else if (DisplayState == EEduMatchModeWidgetState::WaitingForOtherPlayer)
	{
		Description = NSLOCTEXT("EduFPS", "WaitingForOtherPlayerDescription",
			"槽位已锁定。另一名玩家完成选位后，比赛将自动开始。");
	}

	TSharedRef<SVerticalBox> Content = SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.0f, 0.0f, 0.0f, 12.0f)
		[
			SNew(STextBlock)
			.Text(Title)
			.Font(FCoreStyle::GetDefaultFontStyle("Bold", 28))
			.ColorAndOpacity(FLinearColor::White)
		]
		+ SVerticalBox::Slot()
		.AutoHeight()
		.HAlign(HAlign_Center)
		.Padding(0.0f, 0.0f, 0.0f, 18.0f)
		[
			SNew(STextBlock)
			.Text(Description)
			.AutoWrapText(true)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FLinearColor(0.85f, 0.85f, 0.85f, 1.0f))
		];

	if (DisplayState == EEduMatchModeWidgetState::HostSelection)
	{
		Content->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(0.0f, 4.0f)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.ContentPadding(FMargin(18.0f, 10.0f))
			.OnClicked_UObject(this, &UEduMatchModeWidget::SelectSinglePlayer)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("EduFPS", "SinglePlayerMode", "单人模式"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
			]
		];

		Content->AddSlot()
		.AutoHeight()
		.HAlign(HAlign_Fill)
		.Padding(0.0f, 4.0f)
		[
			SNew(SButton)
			.HAlign(HAlign_Center)
			.ContentPadding(FMargin(18.0f, 10.0f))
			.OnClicked_UObject(this, &UEduMatchModeWidget::SelectTwoPlayer)
			[
				SNew(STextBlock)
				.Text(NSLOCTEXT("EduFPS", "TwoPlayerMode", "双人模式"))
				.Font(FCoreStyle::GetDefaultFontStyle("Bold", 20))
			]
		];
	}

	return SNew(SBox)
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		[
			SNew(SBorder)
			.Padding(FMargin(32.0f, 24.0f))
			.BorderBackgroundColor(FLinearColor(0.025f, 0.03f, 0.05f, 0.94f))
			[
				SNew(SBox)
				.WidthOverride(520.0f)
				[
					Content
				]
			]
		];
}

FReply UEduMatchModeWidget::SelectSinglePlayer()
{
	if (AShooterPlayerController* Controller = Cast<AShooterPlayerController>(GetOwningPlayer()))
	{
		Controller->SelectMatchMode(EEduMatchMode::SinglePlayer);
	}
	return FReply::Handled();
}

FReply UEduMatchModeWidget::SelectTwoPlayer()
{
	if (AShooterPlayerController* Controller = Cast<AShooterPlayerController>(GetOwningPlayer()))
	{
		Controller->SelectMatchMode(EEduMatchMode::TwoPlayer);
	}
	return FReply::Handled();
}
