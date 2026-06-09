#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EduMatchModeWidget.generated.h"

class FReply;

UENUM()
enum class EEduMatchModeWidgetState : uint8
{
	HostSelection,
	WaitingForHost,
	WaitingForOtherPlayer,
	SinglePlayerUnavailable
};

/**
 * Native setup screen shown before team-slot selection.
 */
UCLASS()
class TENCENT_EDU_FPS_API UEduMatchModeWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetDisplayState(EEduMatchModeWidgetState NewState);

protected:
	virtual TSharedRef<SWidget> RebuildWidget() override;

private:
	FReply SelectSinglePlayer();
	FReply SelectTwoPlayer();

	EEduMatchModeWidgetState DisplayState = EEduMatchModeWidgetState::WaitingForHost;
};
