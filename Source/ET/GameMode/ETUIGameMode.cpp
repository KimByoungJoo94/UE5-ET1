#include "GameMode/ETUIGameMode.h"
#include "Blueprint/UserWidget.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/GameplayStatics.h"

AETUIGameMode::AETUIGameMode()
{
	DefaultPawnClass = nullptr;
}

void AETUIGameMode::BeginPlay()
{
	Super::BeginPlay();

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController == nullptr || WidgetClass == nullptr)
	{
		return;
	}

	UUserWidget* Widget = CreateWidget<UUserWidget>(PlayerController, WidgetClass);
	Widget->AddToViewport();

	FInputModeUIOnly InputMode;
	InputMode.SetWidgetToFocus(Widget->TakeWidget());
	InputMode.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	PlayerController->SetInputMode(InputMode);
	PlayerController->SetShowMouseCursor(true);
}
