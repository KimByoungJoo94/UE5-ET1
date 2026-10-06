#include "UI/ETChargeAttackWidget.h"
#include "Components/ProgressBar.h"
#include "Components/TextBlock.h"
#include "Components/ETChargeAttackComponent.h"

void UETChargeAttackWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (UETChargeAttackComponent* OwningChargeAttackComponent = GetOwningChargeAttackComponent())
	{
		OnChargeCountChanged(OwningChargeAttackComponent->GetCurrentChargeCount(), OwningChargeAttackComponent->GetMaxChargeCount());
		OnChargeCountChangedHandle = OwningChargeAttackComponent->OnChargeCountChanged.AddUObject(this, &ThisClass::OnChargeCountChanged);
	}
}

void UETChargeAttackWidget::NativeDestruct()
{
	if (UETChargeAttackComponent* OwningChargeAttackComponent = GetOwningChargeAttackComponent())
	{
		OwningChargeAttackComponent->OnChargeCountChanged.Remove(OnChargeCountChangedHandle);
	}

	Super::NativeDestruct();
}

void UETChargeAttackWidget::OnChargeCountChanged(const int32 InCurrentChargeCount, const int32 InMaxChargeCount)
{
	const float ChargePercent = InMaxChargeCount > 0 ? static_cast<float>(InCurrentChargeCount) / InMaxChargeCount : 0.f;

	ProgressBar->SetPercent(ChargePercent);
	CountText->SetText(FText::Format(INVTEXT("{0}/{1}"), FText::AsNumber(InCurrentChargeCount), FText::AsNumber(InMaxChargeCount)));
}
