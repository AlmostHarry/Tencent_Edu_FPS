#pragma once

#include "CoreMinimal.h"
#include "EduTeamSlotTypes.generated.h"

UENUM(BlueprintType)
enum class EEduTeam : uint8
{
	Red = 0,
	Blue = 1,
	Unassigned = 255 UMETA(Hidden)
};

USTRUCT(BlueprintType)
struct FEduTeamSlotSelection
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category="Shooter|Team")
	EEduTeam Team = EEduTeam::Unassigned;

	UPROPERTY(BlueprintReadOnly, Category="Shooter|Team")
	int32 SlotIndex = 0;

	bool IsValid() const
	{
		return Team != EEduTeam::Unassigned && (SlotIndex == 1 || SlotIndex == 2);
	}
};
