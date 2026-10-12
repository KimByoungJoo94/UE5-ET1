#include "UI/ETStatWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/ETGameStatComponent.h"
#include "Math/ColorList.h"

void UETStatWidget::SetGameStatType(const EETGameStatType InGameStatType)
{
	GameStatType = InGameStatType;

	BindGameStatComponent();
}

void UETStatWidget::SetUseStatText(const bool bInUseStatText)
{
	bUseStatText = bInUseStatText;

	UpdateStatTextVisibility();
}

void UETStatWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// 디자이너 미리보기에도 반영
	if (ValueProgressBar)
	{
		ValueProgressBar->SetFillColorAndOpacity(FLinearColor(GColorList.GetFColorByName(*ProgressBarColorName.ToString())));
	}

	UpdateStatTextVisibility();
}

void UETStatWidget::NativeConstruct()
{
	Super::NativeConstruct();

	BindGameStatComponent();
}

void UETStatWidget::NativeDestruct()
{
	UnbindGameStatComponent();

	Super::NativeDestruct();
}

void UETStatWidget::BindGameStatComponent()
{
	// 타입이 바뀌었을 수 있으므로 기존 바인딩은 항상 먼저 해제
	UnbindGameStatComponent();

	if (GameStatType == EETGameStatType::Max)
	{
		return;
	}

	if (UETGameStatComponent* OwningGameStatComponent = GetOwningGameStatComponent())
	{
		BoundGameStatComponent = OwningGameStatComponent;
		OnUpdateGameStatHandle = OwningGameStatComponent->GetOnUpdateGameStatDelegate(GameStatType).AddUObject(this, &ThisClass::OnUpdateGameStat);

		UpdateGameStat(OwningGameStatComponent->GetGameStat(GameStatType));
	}
}

void UETStatWidget::UnbindGameStatComponent()
{
	// 소유 캐릭터가 바뀌어도 구독했던 컴포넌트에서 해제
	if (UETGameStatComponent* GameStatComponent = BoundGameStatComponent.Get())
	{
		GameStatComponent->GetOnUpdateGameStatDelegate(GameStatType).Remove(OnUpdateGameStatHandle);
	}

	BoundGameStatComponent.Reset();
	OnUpdateGameStatHandle.Reset();
}

void UETStatWidget::UpdateStatTextVisibility()
{
	ValueText->SetVisibility(bUseStatText ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
}

void UETStatWidget::OnUpdateGameStat(const FETGameStat& InGameStat)
{
	UpdateGameStat(InGameStat);
}

void UETStatWidget::UpdateGameStat(const FETGameStat& InGameStat)
{
	const float CurrentValue = InGameStat.GetCurrentValue();
	const float MaxValue = InGameStat.GetMaxValue();

	ValueProgressBar->SetPercent(MaxValue > 0.f ? CurrentValue / MaxValue : 0.f);
	
	if (bUseStatText)
	{
		ValueText->SetText(FText::Format(INVTEXT("{0} / {1}"), FText::AsNumber(FMath::FloorToInt32(CurrentValue)), FText::AsNumber(FMath::FloorToInt32(MaxValue))));
	}
}

TArray<FString> UETStatWidget::GetColorNameOptions() const
{
	TArray<FString> ColorNameArray;
	ColorNameArray.Reserve(GColorList.GetColorsNum());

	for (int32 Index = 0; Index < GColorList.GetColorsNum(); ++Index)
	{
		ColorNameArray.Add(GColorList.GetColorNameByIndex(Index));
	}

	return ColorNameArray;
}
