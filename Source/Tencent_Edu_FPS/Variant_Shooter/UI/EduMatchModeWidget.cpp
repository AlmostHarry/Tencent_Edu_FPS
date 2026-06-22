#include "Variant_Shooter/UI/EduMatchModeWidget.h"

#include "Components/Button.h"
#include "Components/TextBlock.h"
#include "EduShooterGameState.h"
#include "ShooterPlayerController.h"

void UEduMatchModeWidget::SetDisplayState(
	EEduMatchModeWidgetState NewState,
	int32 InExpectedHumanPlayerCount)
{
	DisplayState = NewState;
	ExpectedHumanPlayerCount = InExpectedHumanPlayerCount;
	RefreshDisplay();
}

void UEduMatchModeWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SinglePlayerButton->OnClicked.AddUniqueDynamic(this, &UEduMatchModeWidget::SelectSinglePlayer);
	TwoPlayerButton->OnClicked.AddUniqueDynamic(this, &UEduMatchModeWidget::SelectTwoPlayer);

	RefreshDisplay();
}

void UEduMatchModeWidget::RefreshDisplay()
{
	if (!TitleText || !DescriptionText || !SinglePlayerButton || !TwoPlayerButton)
	{
		return;
	}

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
		if (ExpectedHumanPlayerCount == 1)
		{
			Description = NSLOCTEXT("EduFPS", "ChooseSinglePlayerDescription",
				"当前运行配置为1名玩家。选择单人模式后加入3名AI。");
		}
		else if (ExpectedHumanPlayerCount == 2)
		{
			Description = NSLOCTEXT("EduFPS", "ChooseTwoPlayerDescription",
				"当前运行配置为2名玩家。双方完成选位后加入2名AI。");
		}
		else
		{
			Description = NSLOCTEXT("EduFPS", "UnsupportedPlayerCountDescription",
				"当前模式仅支持将PIE玩家数量设置为1或2。");
		}
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

	TitleText->SetText(Title);
	DescriptionText->SetText(Description);

	const bool bHostSelection = DisplayState == EEduMatchModeWidgetState::HostSelection;
	SinglePlayerButton->SetVisibility(
		bHostSelection && ExpectedHumanPlayerCount == 1
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);
	TwoPlayerButton->SetVisibility(
		bHostSelection && ExpectedHumanPlayerCount == 2
			? ESlateVisibility::Visible
			: ESlateVisibility::Collapsed);
}

void UEduMatchModeWidget::SelectSinglePlayer()
{
	if (AShooterPlayerController* Controller = Cast<AShooterPlayerController>(GetOwningPlayer()))
	{
		Controller->SelectMatchMode(EEduMatchMode::SinglePlayer);
	}
}

void UEduMatchModeWidget::SelectTwoPlayer()
{
	if (AShooterPlayerController* Controller = Cast<AShooterPlayerController>(GetOwningPlayer()))
	{
		Controller->SelectMatchMode(EEduMatchMode::TwoPlayer);
	}
}
