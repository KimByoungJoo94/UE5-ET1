#include "UI/ETMenuWidget.h"
#include "UI/ETTextButtonWidget.h"
#include "UI/ETStatListWidget.h"
#include "Controller/ETPlayerController.h"
#include "Components/ETGameStatComponent.h"
#include "Subsystem/ETSaveSubsystem.h"
#include "Subsystem/ETLevelSubsystem.h"
#include "Kismet/KismetSystemLibrary.h"

UETMenuWidget::UETMenuWidget(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	CloseKeyArray.Add(EKeys::Escape);
}

void UETMenuWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetIsFocusable(true);
}

void UETMenuWidget::NativeConstruct()
{
	Super::NativeConstruct();

	ReturnToGameButton->OnClicked.AddUObject(this, &ThisClass::OnReturnToGameButtonClicked);
	ReturnToTitleButton->OnClicked.AddUObject(this, &ThisClass::OnReturnToTitleButtonClicked);
	QuitGameButton->OnClicked.AddUObject(this, &ThisClass::OnQuitGameButtonClicked);

	BuildStatList();
}

void UETMenuWidget::NativeDestruct()
{
	ReturnToGameButton->OnClicked.RemoveAll(this);
	ReturnToTitleButton->OnClicked.RemoveAll(this);
	QuitGameButton->OnClicked.RemoveAll(this);

	Super::NativeDestruct();
}

FReply UETMenuWidget::NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent)
{
	if (CloseKeyArray.Contains(InKeyEvent.GetKey()))
	{
		CloseMenuWidget();
		return FReply::Handled();
	}

	return Super::NativeOnKeyDown(InGeometry, InKeyEvent);
}

void UETMenuWidget::OnReturnToGameButtonClicked()
{
	CloseMenuWidget();
}

void UETMenuWidget::OnReturnToTitleButtonClicked()
{
	// 마지막 저장 이후 얻은 진행(보상 등)을 남기고 이동 (입력 모드는 타이틀 GameMode 가 설정)
	ETSubsystem::Get<UETSaveSubsystem>(this).Save();
	ETSubsystem::Get<UETLevelSubsystem>(this).OpenTitleLevel();
}

void UETMenuWidget::OnQuitGameButtonClicked()
{
	// 저장은 세이브 서브시스템 종료 시 동기 저장
	UKismetSystemLibrary::QuitGame(this, GetOwningPlayer(), EQuitPreference::Quit, false);
}

void UETMenuWidget::BuildStatList()
{
	const UETGameStatComponent* GameStatComponent = GetOwningGameStatComponent();
	if (GameStatComponent == nullptr)
	{
		return;
	}

	FETStatListSettings StatListSettings;
	StatListSettings.StatWidgetClass = StatWidgetClass;
	StatListSettings.StatWidgetSize = FVector2D::ZeroVector;
	StatListSettings.bUseStatText = true;
	StatListSettings.StatTextFormat = EETStatTextFormat::Max;
	StatListSettings.bHideZeroMaxStat = true;

	TArray<EETGameStatType> GameStatTypeArray;
	GameStatComponent->GetGameStatMap().GenerateKeyArray(GameStatTypeArray);
	GameStatTypeArray.Sort();

	for (const EETGameStatType GameStatType : GameStatTypeArray)
	{
		StatListSettings.StatUISettingDataArray.Add(FETStatUISettingData(GameStatType, NAME_None));
	}

	StatListWidget->SetOwningETCharacter(GetOwningETCharacter());
	StatListWidget->BuildStatWidgets(StatListSettings);
}

void UETMenuWidget::CloseMenuWidget()
{
	if (AETPlayerController* PlayerController = Cast<AETPlayerController>(GetOwningPlayer()))
	{
		PlayerController->CloseMenuWidget();
	}
}
