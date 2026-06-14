// Copyright Epic Games, Inc. All Rights Reserved.

#include "EduRespawnCountdownWidget.h"
#include "Components/TextBlock.h"

void UEduRespawnCountdownWidget::SetRemainingSeconds(int32 RemainingSeconds)
{
	const int32 ClampedSeconds = FMath::Max(0, RemainingSeconds);
	if (CountdownText)
	{
		const FText CountdownFormat = NSLOCTEXT(
			"EduRespawnCountdown",
			"CountdownFormat",
			"Respawning in {0}");
		CountdownText->SetText(FText::Format(CountdownFormat, FText::AsNumber(ClampedSeconds)));
	}

	BP_OnCountdownUpdated(ClampedSeconds);
}
