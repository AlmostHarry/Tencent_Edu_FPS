#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EduMatchResultWidget.generated.h"

class SWidget;

/**
 * Minimal native overlay shown when a team reaches the winning score.
 */
UCLASS()
class TENCENT_EDU_FPS_API UEduMatchResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	void SetMatchWon(bool bWon);

protected:

	virtual TSharedRef<SWidget> RebuildWidget() override;

private:

	FReply RestartMatch();

	bool bMatchWon = false;
};
