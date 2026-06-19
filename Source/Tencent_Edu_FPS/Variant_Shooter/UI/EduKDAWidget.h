#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "EduKDAWidget.generated.h"

/**
 * Local HUD widget that displays the owning player's KDA.
 */
UCLASS()
class TENCENT_EDU_FPS_API UEduKDAWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Updates the displayed local player KDA. */
	void SetKDA(int32 Kills, int32 Deaths, int32 Assists);

	UFUNCTION(BlueprintPure, Category="Shooter|KDA")
	int32 GetKills() const { return CachedKills; }

	UFUNCTION(BlueprintPure, Category="Shooter|KDA")
	int32 GetDeaths() const { return CachedDeaths; }

	UFUNCTION(BlueprintPure, Category="Shooter|KDA")
	int32 GetAssists() const { return CachedAssists; }

	UFUNCTION(BlueprintPure, Category="Shooter|KDA")
	FText GetKDAText() const;

protected:
	virtual void NativeConstruct() override;

	/** Blueprint hook for styling, animation, or alternate sub-widget updates. */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter|KDA", meta=(DisplayName="KDA Updated"))
	void BP_OnKDAUpdated(int32 Kills, int32 Deaths, int32 Assists, const FText& DisplayText);

private:
	int32 CachedKills = 0;
	int32 CachedDeaths = 0;
	int32 CachedAssists = 0;

	void BroadcastKDAUpdated();
};
