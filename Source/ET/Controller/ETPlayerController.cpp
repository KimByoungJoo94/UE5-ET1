#include "Controller/ETPlayerController.h"
#include "UI/ETPlayerHUDWidget.h"
#include "UI/ETCheckpointWidget.h"
#include "Components/ETInteractionComponent.h"
#include "Common/ETCommonSettings.h"
#include "Character/ETPlayer.h"
#include "Subsystem/ETSpawnSubsystem.h"

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

void AETPlayerController::OpenCheckpointWidget(const FName InCheckpointRowName)
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

	CheckpointWidget->SetCheckpointRowName(InCheckpointRowName);
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

	// 무릎 꿇고 대기 중이던 상호작용 몽타주 재개
	if (UETInteractionComponent* InteractionComponent = GetPawn() ? GetPawn()->FindComponentByClass<UETInteractionComponent>() : nullptr)
	{
		InteractionComponent->EndInteraction();
	}
}

void AETPlayerController::RestAtCheckpoint()
{
	if (AETPlayer* PlayerCharacter = Cast<AETPlayer>(GetPawn()))
	{
		PlayerCharacter->ResetGameStat();
	}

	ETSubsystem::Get<UETSpawnSubsystem>(this).ResetAllSpawnVolumes();
}
