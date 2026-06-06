#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EduTeamSelectionWidget.generated.h"

class SWidget;

/**
 * Temporary native team-slot selector used before the multiplayer lobby exists.
 */
UCLASS()
class TENCENT_EDU_FPS_API UEduTeamSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual TSharedRef<SWidget> RebuildWidget() override;

private:

	FReply SelectRedSlotOne();
	FReply SelectRedSlotTwo();
	FReply SelectBlueSlotOne();
	FReply SelectBlueSlotTwo();
	FReply SelectSlot(uint8 TeamValue, int32 SlotIndex);
};
