#pragma once

#include "CoreMinimal.h"
#include "EduTeamSlotTypes.h"
#include "GameFramework/PlayerState.h"
#include "EduShooterPlayerState.generated.h"

/**
 * Replicated identity for a human player's selected team and match slot.
 */
UCLASS()
class TENCENT_EDU_FPS_API AEduShooterPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	void SetTeamSlotSelection(const FEduTeamSlotSelection& NewSelection);
	void ClearTeamSlotSelection();

	const FEduTeamSlotSelection& GetTeamSlotSelection() const { return TeamSlotSelection; }
	bool HasSelectedTeamSlot() const { return TeamSlotSelection.IsValid(); }

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

private:
	UPROPERTY(Replicated)
	FEduTeamSlotSelection TeamSlotSelection;
};
