#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EduTeamSelectionWidget.generated.h"

class UButton;

/**
 * UMG base class for the local team-slot selector.
 */
UCLASS()
class TENCENT_EDU_FPS_API UEduTeamSelectionWidget : public UUserWidget
{
	GENERATED_BODY()

protected:

	virtual void NativeConstruct() override;

private:

	UFUNCTION()
	void SelectRedSlotOne();

	UFUNCTION()
	void SelectRedSlotTwo();

	UFUNCTION()
	void SelectBlueSlotOne();

	UFUNCTION()
	void SelectBlueSlotTwo();

	void SelectSlot(uint8 TeamValue, int32 SlotIndex);

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> RedSlotOneButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> RedSlotTwoButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> BlueSlotOneButton;

	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> BlueSlotTwoButton;
};
