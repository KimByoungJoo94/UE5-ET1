#include "UI/ETStatWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/ETGameStatComponent.h"
#include "Math/ColorList.h"

void UETStatWidget::SetGameStatType(const EETGameStatType InGameStatType)
{
	GameStatType = InGameStatType;

	UpdateDisplayNameText();
	BindGameStatComponent();
}

void UETStatWidget::SetUseStatText(const bool bInUseStatText)
{
	bUseStatText = bInUseStatText;

	UpdateStatTextVisibility();
}

void UETStatWidget::SetStatTextFormat(const EETStatTextFormat InStatTextFormat)
{
	StatTextFormat = InStatTextFormat;

	if (const UETGameStatComponent* GameStatComponent = BoundGameStatComponent.Get())
	{
		UpdateGameStat(GameStatComponent->GetGameStat(GameStatType));
	}
}

void UETStatWidget::SetProgressBarColorName(const FName InProgressBarColorName)
{
	ProgressBarColorName = InProgressBarColorName;

	UpdateProgressBarColor();
}

void UETStatWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// 디자이너 미리보기에도 반영
	UpdateProgressBarColor();
	UpdateStatTextVisibility();
	UpdateDisplayNameText();
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

void UETStatWidget::UpdateProgressBarColor()
{
	if (ValueProgressBar)
	{
		ValueProgressBar->SetFillColorAndOpacity(FLinearColor(GColorList.GetFColorByName(*ProgressBarColorName.ToString())));
	}
}

void UETStatWidget::UpdateDisplayNameText()
{
	if (DisplayNameText && GameStatType != EETGameStatType::Max)
	{
		DisplayNameText->SetText(StaticEnum<EETGameStatType>()->GetDisplayNameTextByValue(static_cast<int64>(GameStatType)));
	}
}

void UETStatWidget::OnUpdateGameStat(const FETGameStat& InGameStat)
{
	UpdateGameStat(InGameStat);
}

void UETStatWidget::UpdateGameStat(const FETGameStat& InGameStat)
{
	const float CurrentValue = InGameStat.GetCurrentValue();
	const float MaxValue = InGameStat.GetMaxValue();

	if (ValueProgressBar)
	{
		ValueProgressBar->SetPercent(MaxValue > 0.f ? CurrentValue / MaxValue : 0.f);
	}

	if (bUseStatText == false)
	{
		return;
	}

	const FText CurrentValueText = FText::AsNumber(FMath::FloorToInt32(CurrentValue));
	const FText MaxValueText = FText::AsNumber(FMath::FloorToInt32(MaxValue));
	switch (StatTextFormat)
	{
	case EETStatTextFormat::Current:
		ValueText->SetText(CurrentValueText);
		break;
	case EETStatTextFormat::Max:
		ValueText->SetText(MaxValueText);
		break;
	default:
		ValueText->SetText(FText::Format(INVTEXT("{0} / {1}"), CurrentValueText, MaxValueText));
		break;
	}
}
