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

	int32 StatWidgetNum = 0;
	for (const FETStatBarData& StatBarData : InStatListSettings.StatBarDataArray)
	{
		if (GameStatComponent->GetGameStat(StatBarData.GameStatType).GetMaxValue() <= 0.f)
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
		StatWidget->SetProgressBarColorName(StatBarData.ProgressBarColorName);
		StatWidget->SetGameStatType(StatBarData.GameStatType);

		// WBP 크기와 상관없이 설정 크기로 고정
		USizeBox* SizeBox = WidgetTree->ConstructWidget<USizeBox>(USizeBox::StaticClass());
		SizeBox->SetWidthOverride(InStatListSettings.StatBarSize.X);
		SizeBox->SetHeightOverride(InStatListSettings.StatBarSize.Y);
		SizeBox->AddChild(StatWidget);

		UVerticalBoxSlot* VerticalBoxSlot = StatVerticalBox->AddChildToVerticalBox(SizeBox);
		VerticalBoxSlot->SetPadding(FMargin(0.f, StatWidgetNum > 0 ? InStatListSettings.StatBarSpacing : 0.f, 0.f, 0.f));

		++StatWidgetNum;
	}

	const float TotalHeight = StatWidgetNum * InStatListSettings.StatBarSize.Y + FMath::Max(StatWidgetNum - 1, 0) * InStatListSettings.StatBarSpacing;
	return FVector2D(InStatListSettings.StatBarSize.X, TotalHeight);
}
