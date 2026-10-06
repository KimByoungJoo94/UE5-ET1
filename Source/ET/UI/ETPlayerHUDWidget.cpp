#include "UI/ETPlayerHUDWidget.h"
#include "Components/ProgressBar.h"
#include "Components/ETGameStatComponent.h"
#include "Character/ETCharacter.h"

void UETPlayerHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	UpdateHealth();

	if (UETGameStatComponent* OwningGameStatComponent = GetOwningGameStatComponent())
	{
		OnUpdateGameStatHandle = OwningGameStatComponent->OnUpdateGameStat.AddUObject(this, &ThisClass::OnUpdateGameStat);
	}
}

void UETPlayerHUDWidget::NativeDestruct()
{
	if (UETGameStatComponent* OwningGameStatComponent = GetOwningGameStatComponent())
	{
		OwningGameStatComponent->OnUpdateGameStat.Remove(OnUpdateGameStatHandle);
	}

	Super::NativeDestruct();
}

void UETPlayerHUDWidget::UpdateHealth()
{	
	if (UETGameStatComponent* OwningGameStatComponent = GetOwningGameStatComponent())
	{
		const FETGameStat& HealthStat = OwningGameStatComponent->GetGameStat(EETGameStatType::Health);
		const float HealthPercent = HealthStat.GetCurrentValue() / HealthStat.GetMaxValue();

		HealthProgressBar->SetPercent(HealthPercent);
	}
}

void UETPlayerHUDWidget::OnUpdateGameStat(const FETGameStat& InGameStat)
{

}
