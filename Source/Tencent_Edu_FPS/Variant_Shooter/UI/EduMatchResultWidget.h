#pragma once

#include "CoreMinimal.h"
#include "EduShooterPlayerState.h"
#include "Blueprint/UserWidget.h"
#include "EduMatchResultWidget.generated.h"

class UButton;
class UTextBlock;

/**
 * UMG base class for the match result screen.
 */
UCLASS()
class TENCENT_EDU_FPS_API UEduMatchResultWidget : public UUserWidget
{
	GENERATED_BODY()

public:

	/** Updates the result displayed by the widget */
	void SetMatchWon(bool bWon);

	/** Updates the local player's final KDA summary */
	void SetPlayerKDA(const FEduPlayerMatchStats& Stats);

	/** Restarts the current map and returns to team selection */
	UFUNCTION(BlueprintCallable, Category="Match")
	void RestartMatch();

protected:

	virtual void NativeConstruct() override;

	/** Allows a Widget Blueprint to override result styling and animation */
	UFUNCTION(BlueprintNativeEvent, Category="Match", meta=(DisplayName="Set Match Result"))
	void BP_SetMatchResult(bool bWon);
	virtual void BP_SetMatchResult_Implementation(bool bWon);

	/** WIN/LOSE label supplied by the Widget Blueprint */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> ResultText;

	/** Restart button supplied by the Widget Blueprint */
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UButton> RestartButton;

	/** Blueprint hook for custom final KDA presentation. */
	UFUNCTION(BlueprintImplementableEvent, Category="Match", meta=(DisplayName="Player KDA Updated"))
	void BP_OnPlayerKDAUpdated(int32 Kills, int32 Deaths, int32 Assists, const FText& DisplayText);

private:
	FEduPlayerMatchStats CachedStats;

	FText GetPlayerKDAText() const;
	void BroadcastPlayerKDAUpdated();
};
