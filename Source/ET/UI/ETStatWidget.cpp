#include "UI/ETStatWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/ETGameStatComponent.h"
#include "Math/ColorList.h"

void UETStatWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// 디자이너 미리보기에도 반영
	if (ValueProgressBar)
	{
		ValueProgressBar->SetFillColorAndOpacity(FLinearColor(GColorList.GetFColorByName(*ProgressBarColorName.ToString())));
	}
}

void UETStatWidget::NativeConstruct()
{
	Super::NativeConstruct();

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

void UETStatWidget::NativeDestruct()
{
	// 소유 캐릭터가 바뀌어도 구독했던 컴포넌트에서 해제
	if (UETGameStatComponent* GameStatComponent = BoundGameStatComponent.Get())
	{
		GameStatComponent->GetOnUpdateGameStatDelegate(GameStatType).Remove(OnUpdateGameStatHandle);
	}

	BoundGameStatComponent.Reset();
	OnUpdateGameStatHandle.Reset();

	Super::NativeDestruct();
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
	ValueText->SetText(FText::Format(INVTEXT("{0} / {1}"), FText::AsNumber(FMath::FloorToInt32(CurrentValue)), FText::AsNumber(FMath::FloorToInt32(MaxValue))));
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
