#include "UI/ETCheckpointTravelWidget.h"
#include "UI/ETTextButtonWidget.h"
#include "Controller/ETPlayerController.h"
#include "Components/PanelWidget.h"
#include "DataAsset/ETLevelTableRowData.h"
#include "DataAsset/ETCheckpointTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Subsystem/ETSaveSubsystem.h"
#include "Subsystem/ETLevelSubsystem.h"

void UETCheckpointTravelWidget::OpenTravel(const FName InCurrentCheckpointRowName)
{
	CurrentCheckpointRowName = InCurrentCheckpointRowName;
	SelectedCheckpointRowName = InCurrentCheckpointRowName;

	const FETCheckpointTableRowData* CheckpointRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETCheckpointTableRowData>(CurrentCheckpointRowName);
	SelectedLevelRowName = CheckpointRowData ? CheckpointRowData->LevelRowName : NAME_None;

	RebuildLevelButtons();
	RebuildCheckpointButtons();
	UpdateButtonBackgroundColors();

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

void UETCheckpointTravelWidget::CloseTravel()
{
	SetVisibility(ESlateVisibility::Collapsed);
}

bool UETCheckpointTravelWidget::IsTravelOpen() const
{
	return GetVisibility() != ESlateVisibility::Collapsed && GetVisibility() != ESlateVisibility::Hidden;
}

void UETCheckpointTravelWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BackButton->OnClicked.AddUObject(this, &ThisClass::OnBackButtonClicked);
}

void UETCheckpointTravelWidget::NativeDestruct()
{
	BackButton->OnClicked.RemoveAll(this);

	// 다시 열 때는 OpenTravel 에서 새로 만듦
	ClearButtons(LevelButtonPanel, LevelButtonMap);
	ClearButtons(CheckpointButtonPanel, CheckpointButtonMap);

	Super::NativeDestruct();
}

void UETCheckpointTravelWidget::OnBackButtonClicked()
{
	CloseTravel();
}

void UETCheckpointTravelWidget::OnLevelButtonClicked(FName InLevelRowName)
{
	if (InLevelRowName == SelectedLevelRowName)
	{
		return;
	}

	SelectedLevelRowName = InLevelRowName;

	// 현재 체크포인트가 있는 레벨로 돌아오면 현재 체크포인트를 다시 선택
	const FETCheckpointTableRowData* CurrentCheckpointRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETCheckpointTableRowData>(CurrentCheckpointRowName);
	SelectedCheckpointRowName = CurrentCheckpointRowData && CurrentCheckpointRowData->LevelRowName == SelectedLevelRowName ? CurrentCheckpointRowName : NAME_None;

	RebuildCheckpointButtons();
	UpdateButtonBackgroundColors();
}

void UETCheckpointTravelWidget::OnCheckpointButtonClicked(FName InCheckpointRowName)
{
	if (InCheckpointRowName != SelectedCheckpointRowName)
	{
		SelectedCheckpointRowName = InCheckpointRowName;

		UpdateButtonBackgroundColors();
		return;
	}

	// 선택된 상태에서 한 번 더 클릭하면 이동 (현재 체크포인트는 이동하지 않음)
	if (InCheckpointRowName == CurrentCheckpointRowName)
	{
		return;
	}

	// UI 입력 모드가 다음 레벨까지 남지 않도록 체크포인트 창을 닫고 이동
	if (AETPlayerController* PlayerController = Cast<AETPlayerController>(GetOwningPlayer()))
	{
		PlayerController->CloseCheckpointWidget();
	}

	ETSubsystem::Get<UETLevelSubsystem>(this).TravelToCheckpoint(InCheckpointRowName);
}

void UETCheckpointTravelWidget::RebuildLevelButtons()
{
	ClearButtons(LevelButtonPanel, LevelButtonMap);

	const UETGameDataSubsystem& GameDataSubsystem = ETSubsystem::Get<UETGameDataSubsystem>(this);
	const UDataTable* LevelDataTable = GameDataSubsystem.GetDataTable<FETLevelTableRowData>();
	const UDataTable* CheckpointDataTable = GameDataSubsystem.GetDataTable<FETCheckpointTableRowData>();
	if (LevelDataTable == nullptr || CheckpointDataTable == nullptr)
	{
		return;
	}

	// 발견한 체크포인트가 있는 레벨
	const TSet<FName>& DiscoveredCheckpointRowNameSet = ETSubsystem::Get<UETSaveSubsystem>(this).GetWorldSaveData().DiscoveredCheckpointRowNameSet;
	TSet<FName> DiscoveredLevelRowNameSet;
	for (const TPair<FName, uint8*>& CheckpointRowPair : CheckpointDataTable->GetRowMap())
	{
		if (DiscoveredCheckpointRowNameSet.Contains(CheckpointRowPair.Key))
		{
			DiscoveredLevelRowNameSet.Add(reinterpret_cast<const FETCheckpointTableRowData*>(CheckpointRowPair.Value)->LevelRowName);
		}
	}

	for (const TPair<FName, uint8*>& LevelRowPair : LevelDataTable->GetRowMap())
	{
		const FETLevelTableRowData* LevelRowData = reinterpret_cast<const FETLevelTableRowData*>(LevelRowPair.Value);
		if (UETTextButtonWidget* LevelButton = CreateButton(LevelButtonPanel, LevelRowData->DisplayText, DiscoveredLevelRowNameSet.Contains(LevelRowPair.Key)))
		{
			LevelButton->OnClicked.AddUObject(this, &ThisClass::OnLevelButtonClicked, LevelRowPair.Key);
			LevelButtonMap.Add(LevelRowPair.Key, LevelButton);
		}
	}
}

void UETCheckpointTravelWidget::RebuildCheckpointButtons()
{
	ClearButtons(CheckpointButtonPanel, CheckpointButtonMap);

	const UDataTable* CheckpointDataTable = ETSubsystem::Get<UETGameDataSubsystem>(this).GetDataTable<FETCheckpointTableRowData>();
	if (CheckpointDataTable == nullptr)
	{
		return;
	}

	const TSet<FName>& DiscoveredCheckpointRowNameSet = ETSubsystem::Get<UETSaveSubsystem>(this).GetWorldSaveData().DiscoveredCheckpointRowNameSet;

	for (const TPair<FName, uint8*>& CheckpointRowPair : CheckpointDataTable->GetRowMap())
	{
		const FETCheckpointTableRowData* CheckpointRowData = reinterpret_cast<const FETCheckpointTableRowData*>(CheckpointRowPair.Value);
		if (CheckpointRowData->LevelRowName != SelectedLevelRowName)
		{
			continue;
		}

		if (UETTextButtonWidget* CheckpointButton = CreateButton(CheckpointButtonPanel, CheckpointRowData->DisplayText, DiscoveredCheckpointRowNameSet.Contains(CheckpointRowPair.Key)))
		{
			CheckpointButton->OnClicked.AddUObject(this, &ThisClass::OnCheckpointButtonClicked, CheckpointRowPair.Key);
			CheckpointButtonMap.Add(CheckpointRowPair.Key, CheckpointButton);
		}
	}
}

void UETCheckpointTravelWidget::UpdateButtonBackgroundColors()
{
	for (const TPair<FName, TObjectPtr<UETTextButtonWidget>>& LevelButtonPair : LevelButtonMap)
	{
		LevelButtonPair.Value->SetButtonBackgroundColor(LevelButtonPair.Key == SelectedLevelRowName ? SelectedBackgroundColor : NormalBackgroundColor);
	}

	for (const TPair<FName, TObjectPtr<UETTextButtonWidget>>& CheckpointButtonPair : CheckpointButtonMap)
	{
		CheckpointButtonPair.Value->SetButtonBackgroundColor(CheckpointButtonPair.Key == SelectedCheckpointRowName ? SelectedBackgroundColor : NormalBackgroundColor);
	}
}

void UETCheckpointTravelWidget::ClearButtons(UPanelWidget* InPanel, OUT TMap<FName, TObjectPtr<UETTextButtonWidget>>& OutButtonMap)
{
	for (const TPair<FName, TObjectPtr<UETTextButtonWidget>>& ButtonPair : OutButtonMap)
	{
		ButtonPair.Value->OnClicked.RemoveAll(this);
	}

	OutButtonMap.Reset();
	InPanel->ClearChildren();
}

UETTextButtonWidget* UETCheckpointTravelWidget::CreateButton(UPanelWidget* InPanel, const FText& InButtonText, const bool bInEnabled)
{
	if (ButtonClass == nullptr)
	{
		return nullptr;
	}

	UETTextButtonWidget* TextButton = CreateWidget<UETTextButtonWidget>(this, ButtonClass);
	if (TextButton == nullptr)
	{
		return nullptr;
	}

	TextButton->SetButtonText(InButtonText);
	TextButton->SetButtonEnabled(bInEnabled);
	InPanel->AddChild(TextButton);

	return TextButton;
}
