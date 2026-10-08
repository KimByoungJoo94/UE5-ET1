#include "UI/ETCheckpointWidget.h"
#include "Controller/ETPlayerController.h"
#include "Components/Button.h"

UETCheckpointWidget::UETCheckpointWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CloseKeyArray.Add(EKeys::Escape);
}

void UETCheckpointWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);

	CloseButton->OnClicked.AddDynamic(this, &ThisClass::OnCloseButtonClicked);
}

FReply UETCheckpointWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (CloseKeyArray.Contains(InKeyEvent.GetKey()))
	{
		CloseCheckpointWidget();

		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UETCheckpointWidget::OnCloseButtonClicked()
{
	CloseCheckpointWidget();
}

void UETCheckpointWidget::CloseCheckpointWidget()
{
	if (AETPlayerController* PlayerController = Cast<AETPlayerController>(GetOwningPlayer()))
	{
		PlayerController->CloseCheckpointWidget();
	}
}
