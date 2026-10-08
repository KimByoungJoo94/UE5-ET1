#include "Controller/ETPlayerController.h"
#include "UI/ETPlayerHUDWidget.h"
#include "UI/ETCheckpointWidget.h"
#include "Common/ETCommonSettings.h"

void AETPlayerController::BeginPlay()
{
	Super::BeginPlay();

	if (UClass* PlayerHUDWidgetClass = GetDefault<UETCommonSettings>()->UI.PlayerHUDWidgetClass.LoadSynchronous())
	{
		PlayerHUDWidget = CreateWidget<UETPlayerHUDWidget>(GetWorld(), PlayerHUDWidgetClass);
		if (PlayerHUDWidget)
		{
			PlayerHUDWidget->AddToViewport();
		}
	}
}

void AETPlayerController::OpenCheckpointWidget()
{
	if (CheckpointWidget && CheckpointWidget->IsInViewport())
	{
		return;
	}

	if (CheckpointWidget == nullptr)
	{
		if (UClass* CheckpointWidgetClass = GetDefault<UETCommonSettings>()->UI.CheckpointWidgetClass.LoadSynchronous())
		{
			CheckpointWidget = CreateWidget<UETCheckpointWidget>(this, CheckpointWidgetClass);
		}
	}

	if (CheckpointWidget == nullptr)
	{
		return;
	}

	if (PlayerHUDWidget)
	{
		PlayerHUDVisibility = PlayerHUDWidget->GetVisibility();
		PlayerHUDWidget->SetVisibility(ESlateVisibility::Collapsed);
	}

	CheckpointWidget->AddToViewport(1);

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(CheckpointWidget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	SetInputMode(InputMode);
	SetShowMouseCursor(true);
}

void AETPlayerController::CloseCheckpointWidget()
{
	if (CheckpointWidget == nullptr || CheckpointWidget->IsInViewport() == false)
	{
		return;
	}

	CheckpointWidget->RemoveFromParent();

	if (PlayerHUDWidget)
	{
		PlayerHUDWidget->SetVisibility(PlayerHUDVisibility);
	}

	SetInputMode(FInputModeGameOnly());
	SetShowMouseCursor(false);
}
