#pragma once

#include "CoreMinimal.h"
#include "Variant_Shooter/EduShooterGameState.h"
#include "Blueprint/UserWidget.h"
#include "EduScoreboardWidget.generated.h"

class UTextBlock;

/**
 * Local overlay widget that presents the replicated match scoreboard.
 */
UCLASS()
class TENCENT_EDU_FPS_API UEduScoreboardWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	/** Updates the scoreboard entries shown by the widget. */
	void SetScoreboardEntries(const TArray<FEduScoreboardEntry>& Entries);

	UFUNCTION(BlueprintPure, Category="Shooter|Scoreboard")
	TArray<FEduScoreboardEntry> GetScoreboardEntries() const { return CachedEntries; }

	UFUNCTION(BlueprintPure, Category="Shooter|Scoreboard")
	FText GetScoreboardText() const;

protected:
	virtual void NativeConstruct() override;

	/** Blueprint hook for custom table layout, styling, or animation. */
	UFUNCTION(BlueprintImplementableEvent, Category="Shooter|Scoreboard", meta=(DisplayName="Scoreboard Updated"))
	void BP_OnScoreboardUpdated(const TArray<FEduScoreboardEntry>& Entries, const FText& DisplayText);

	/** Optional TextBlock for the simple MVP presentation Blueprint. */
	UPROPERTY(meta=(BindWidgetOptional))
	TObjectPtr<UTextBlock> ScoreboardText;

private:
	UPROPERTY()
	TArray<FEduScoreboardEntry> CachedEntries;

	void BroadcastScoreboardUpdated();
};
