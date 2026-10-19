#include "UI/ETTitleWidget.h"
#include "UI/ETTextButtonWidget.h"
#include "Common/ETCommonSettings.h"
#include "DataAsset/ETLevelTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Subsystem/ETSaveSubsystem.h"
#include "Subsystem/ETLevelSubsystem.h"
#include "GameFramework/PlayerController.h"
#include "Kismet/KismetSystemLibrary.h"

void UETTitleWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ContinueButton->OnClicked.AddUObject(this, &ThisClass::OnContinueButtonClicked);
	NewGameButton->OnClicked.AddUObject(this, &ThisClass::OnNewGameButtonClicked);
	QuitButton->OnClicked.AddUObject(this, &ThisClass::OnQuitButtonClicked);

	ContinueButton->SetVisibility(CanContinue() ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
}

void UETTitleWidget::NativeDestruct()
{
	ContinueButton->OnClicked.RemoveAll(this);
	NewGameButton->OnClicked.RemoveAll(this);
	QuitButton->OnClicked.RemoveAll(this);

	Super::NativeDestruct();
}

void UETTitleWidget::OnContinueButtonClicked()
{
	OpenLevel(ETSubsystem::Get<UETSaveSubsystem>(this).GetWorldSaveData().LevelRowName);
}

void UETTitleWidget::OnNewGameButtonClicked()
{
	ETSubsystem::Get<UETSaveSubsystem>(this).ResetSaveGame();

	OpenLevel(GetDefault<UETCommonSettings>()->Level.StartLevelRowName);
}

void UETTitleWidget::OnQuitButtonClicked()
{
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

bool UETTitleWidget::CanContinue() const
{
	const UETSaveSubsystem& SaveSubsystem = ETSubsystem::Get<UETSaveSubsystem>(this);
	const FName LevelRowName = SaveSubsystem.GetWorldSaveData().LevelRowName;
	if (SaveSubsystem.HasSaveGameInSlot() == false || LevelRowName.IsNone())
	{
		return false;
	}

	return ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETLevelTableRowData>(LevelRowName) != nullptr;
}

void UETTitleWidget::OpenLevel(const FName InLevelRowName)
{
	if (APlayerController* PlayerController = GetOwningPlayer())
	{
		PlayerController->SetInputMode(FInputModeGameOnly());
		PlayerController->SetShowMouseCursor(false);
	}

	ETSubsystem::Get<UETLevelSubsystem>(this).OpenLevel(InLevelRowName);
}
