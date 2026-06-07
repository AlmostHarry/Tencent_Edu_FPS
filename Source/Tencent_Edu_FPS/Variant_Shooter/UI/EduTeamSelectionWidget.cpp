#include "Variant_Shooter/UI/EduTeamSelectionWidget.h"

#include "Components/Button.h"
#include "EduTeamSlotTypes.h"
#include "ShooterPlayerController.h"

void UEduTeamSelectionWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RedSlotOneButton->OnClicked.AddUniqueDynamic(this, &UEduTeamSelectionWidget::SelectRedSlotOne);
	RedSlotTwoButton->OnClicked.AddUniqueDynamic(this, &UEduTeamSelectionWidget::SelectRedSlotTwo);
	BlueSlotOneButton->OnClicked.AddUniqueDynamic(this, &UEduTeamSelectionWidget::SelectBlueSlotOne);
	BlueSlotTwoButton->OnClicked.AddUniqueDynamic(this, &UEduTeamSelectionWidget::SelectBlueSlotTwo);
}

void UEduTeamSelectionWidget::SelectRedSlotOne()
{
	SelectSlot(static_cast<uint8>(EEduTeam::Red), 1);
}

void UEduTeamSelectionWidget::SelectRedSlotTwo()
{
	SelectSlot(static_cast<uint8>(EEduTeam::Red), 2);
}

void UEduTeamSelectionWidget::SelectBlueSlotOne()
{
	SelectSlot(static_cast<uint8>(EEduTeam::Blue), 1);
}

void UEduTeamSelectionWidget::SelectBlueSlotTwo()
{
	SelectSlot(static_cast<uint8>(EEduTeam::Blue), 2);
}

void UEduTeamSelectionWidget::SelectSlot(uint8 TeamValue, int32 SlotIndex)
{
	if (AShooterPlayerController* Controller = Cast<AShooterPlayerController>(GetOwningPlayer()))
	{
		Controller->SelectTeamSlot(static_cast<EEduTeam>(TeamValue), SlotIndex);
	}
}
