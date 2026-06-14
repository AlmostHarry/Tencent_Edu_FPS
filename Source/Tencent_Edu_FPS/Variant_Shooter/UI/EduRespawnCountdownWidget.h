// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EduRespawnCountdownWidget.generated.h"

class UTextBlock;

/**
 * Blueprint base for the local player's respawn countdown overlay.
 * Add a TextBlock named CountdownText to the widget blueprint.
 */
UCLASS(abstract)
class TENCENT_EDU_FPS_API UEduRespawnCountdownWidget : public UUserWidget
{
	GENERATED_BODY()

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> CountdownText;

public:
	/** Updates the visible whole-second countdown. */
	UFUNCTION(BlueprintCallable, Category="Shooter|Respawn")
	void SetRemainingSeconds(int32 RemainingSeconds);

	/** Optional Blueprint hook for animations or additional presentation. */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter|Respawn", meta=(DisplayName="Respawn Countdown Updated"))
	void BP_OnCountdownUpdated(int32 RemainingSeconds);
};
