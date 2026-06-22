#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EduMatchModeWidget.generated.h"

class UButton;
class UTextBlock;

UENUM()
enum class EEduMatchModeWidgetState : uint8
{
	HostSelection,
	WaitingForHost,
	WaitingForOtherPlayer,
	SinglePlayerUnavailable
};

/**
 * UMG base class for the match mode selection screen.
 */
UCLASS()
class TENCENT_EDU_FPS_API UEduMatchModeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetDisplayState(EEduMatchModeWidgetState NewState, int32 InExpectedHumanPlayerCount);

protected:
	virtual void NativeConstruct() override;

private:
	UFUNCTION()
	void SelectSinglePlayer();

	UFUNCTION()
	void SelectTwoPlayer();

	void RefreshDisplay();

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> TitleText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> DescriptionText;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> SinglePlayerButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> TwoPlayerButton;

	EEduMatchModeWidgetState DisplayState = EEduMatchModeWidgetState::WaitingForHost;
	int32 ExpectedHumanPlayerCount = 0;
};
