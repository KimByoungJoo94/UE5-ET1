#include "UI/ETCheckpointWidget.h"
#include "UI/ETTextButtonWidget.h"
#include "UI/ETCheckpointTravelWidget.h"
#include "Controller/ETPlayerController.h"
#include "Components/TextBlock.h"
#include "DataAsset/ETCheckpointTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"

UETCheckpointWidget::UETCheckpointWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CloseKeyArray.Add(EKeys::Escape);
}

void UETCheckpointWidget::SetCheckpointRowName(const FName InCheckpointRowName)
{
	CheckpointRowName = InCheckpointRowName;

	const FETCheckpointTableRowData* CheckpointRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETCheckpointTableRowData>(CheckpointRowName);
	CheckpointNameText->SetText(CheckpointRowData ? CheckpointRowData->DisplayText : FText::GetEmpty());

	TravelWidget->CloseTravel();
}

void UETCheckpointWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);
}

void UETCheckpointWidget::NativeConstruct()
{
	Super::NativeConstruct();

	RestButton->OnClicked.AddUObject(this, &ThisClass::OnRestButtonClicked);
	TravelButton->OnClicked.AddUObject(this, &ThisClass::OnTravelButtonClicked);
	CloseButton->OnClicked.AddUObject(this, &ThisClass::OnCloseButtonClicked);
}

void UETCheckpointWidget::NativeDestruct()
{
	RestButton->OnClicked.RemoveAll(this);
	TravelButton->OnClicked.RemoveAll(this);
	CloseButton->OnClicked.RemoveAll(this);

	Super::NativeDestruct();
}

FReply UETCheckpointWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (CloseKeyArray.Contains(InKeyEvent.GetKey()))
	{
		if (TravelWidget->IsTravelOpen())
		{
			TravelWidget->CloseTravel();
		}
		else
		{
			CloseCheckpointWidget();
		}

		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UETCheckpointWidget::OnRestButtonClicked()
{
	if (AETPlayerController* PlayerController = Cast<AETPlayerController>(GetOwningPlayer()))
	{
		PlayerController->RestAtCheckpoint();
	}
}

void UETCheckpointWidget::OnTravelButtonClicked()
{
	TravelWidget->OpenTravel(CheckpointRowName);
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
