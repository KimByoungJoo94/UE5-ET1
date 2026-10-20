#include "UI/ETStatListWidget.h"
#include "UI/ETStatWidget.h"
#include "Components/ETGameStatComponent.h"
#include "Components/VerticalBox.h"
#include "Components/VerticalBoxSlot.h"
#include "Components/SizeBox.h"
#include "Blueprint/WidgetTree.h"

bool UETStatListWidget::Initialize()
{
	if (Super::Initialize() == false)
	{
		return false;
	}

	// WBP 가 없으므로 루트를 직접 생성
	if (WidgetTree && WidgetTree->RootWidget == nullptr)
	{
		StatVerticalBox = WidgetTree->ConstructWidget<UVerticalBox>(UVerticalBox::StaticClass(), TEXT("StatVerticalBox"));
		WidgetTree->RootWidget = StatVerticalBox;
	}

	return true;
}

FVector2D UETStatListWidget::BuildStatWidgets(const FETStatListSettings& InStatListSettings)
{
	if (StatVerticalBox == nullptr)
	{
		return FVector2D::ZeroVector;
	}

	StatVerticalBox->ClearChildren();

	const UETGameStatComponent* GameStatComponent = GetOwningGameStatComponent();
	const TSubclassOf<UETStatWidget> StatWidgetClass = InStatListSettings.StatWidgetClass.LoadSynchronous();
	if (GameStatComponent == nullptr || StatWidgetClass == nullptr)
	{
		return FVector2D::ZeroVector;
	}

	const bool bFixedSize = InStatListSettings.StatWidgetSize.X > 0.f && InStatListSettings.StatWidgetSize.Y > 0.f;

	int32 StatWidgetNum = 0;
	for (const FETStatUISettingData& StatUISettingData : InStatListSettings.StatUISettingDataArray)
	{
		if (InStatListSettings.bHideZeroMaxStat && GameStatComponent->GetGameStat(StatUISettingData.GameStatType).GetMaxValue() <= 0.f)
		{
			continue;
		}

		UETStatWidget* StatWidget = CreateWidget<UETStatWidget>(this, StatWidgetClass);
		if (StatWidget == nullptr)
		{
			continue;
		}

		StatWidget->SetOwningETCharacter(GetOwningETCharacter());
		StatWidget->SetUseStatText(InStatListSettings.bUseStatText);
		StatWidget->SetStatTextFormat(InStatListSettings.StatTextFormat);
		StatWidget->SetProgressBarColorName(StatUISettingData.ProgressBarColorName);
		StatWidget->SetGameStatType(StatUISettingData.GameStatType);

		UWidget* EntryWidget = StatWidget;
		if (bFixedSize)
		{
			// WBP 크기와 상관없이 설정 크기로 고정
			USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
			SizeBox->SetWidthOverride(InStatListSettings.StatWidgetSize.X);
			SizeBox->SetHeightOverride(InStatListSettings.StatWidgetSize.Y);
			SizeBox->AddChild(StatWidget);
			EntryWidget = SizeBox;
		}

		UVerticalBoxSlot* VerticalBoxSlot = StatVerticalBox->AddChildToVerticalBox(EntryWidget);
		VerticalBoxSlot->SetPadding(FMargin(0.f, StatWidgetNum > 0 ? InStatListSettings.StatWidgetSpacing : 0.f, 0.f, 0.f));

		++StatWidgetNum;
	}

	if (bFixedSize == false)
	{
		return FVector2D::ZeroVector;
	}

	const float TotalHeight = StatWidgetNum * InStatListSettings.StatWidgetSize.Y + FMath::Max(StatWidgetNum - 1, 0) * InStatListSettings.StatWidgetSpacing;
	return FVector2D(InStatListSettings.StatWidgetSize.X, TotalHeight);
}
