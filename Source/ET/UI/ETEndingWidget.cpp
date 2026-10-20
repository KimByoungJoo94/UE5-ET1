#include "UI/ETEndingWidget.h"
#include "UI/ETTextButtonWidget.h"
#include "Subsystem/ETLevelSubsystem.h"
#include "Animation/WidgetAnimation.h"
#include "GameFramework/PlayerController.h"
#include "TimerManager.h"

void UETEndingWidget::NativeConstruct()
{
	Super::NativeConstruct();

	SkipButton->OnClicked.AddUObject(this, &ThisClass::OnSkipButtonClicked);

	if (ShowAnimation)
	{
		PlayAnimation(ShowAnimation);
	}

	if (DisplayDuration > 0.f)
	{
		GetWorld()->GetTimerManager().SetTimer(ReturnToTitleTimerHandle, this, &ThisClass::ReturnToTitle, DisplayDuration, false);
	}
}

void UETEndingWidget::NativeDestruct()
{
	SkipButton->OnClicked.RemoveAll(this);

	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(ReturnToTitleTimerHandle);
	}

	Super::NativeDestruct();
}

void UETEndingWidget::OnSkipButtonClicked()
{
	ReturnToTitle();
}

void UETEndingWidget::ReturnToTitle()
{
	if (bReturningToTitle)
	{
		return;
	}

	bReturningToTitle = true;

	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}

	ETSubsystem::Get<UETLevelSubsystem>(this).OpenTitleLevel();
}
