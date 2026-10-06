#include "Components/ETChargeAttackComponent.h"
#include "Character/ETCharacter.h"
#include "Components/ETGameStatComponent.h"
#include "TimerManager.h"

UETChargeAttackComponent::UETChargeAttackComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UETChargeAttackComponent::StartChargeAttack()
{
	ResetChargeAttack();

	if (GetMaxChargeCount() > 0)
	{
		GetWorld()->GetTimerManager().SetTimer(ChargeTimerHandle, this, &ThisClass::OnChargeTimer, ChargeInterval, true);
	}
}

void UETChargeAttackComponent::EndChargeAttack()
{
	GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);
}

void UETChargeAttackComponent::ResetChargeAttack()
{
	GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);

	CurrentChargeCount = 0;
	OnChargeCountChanged.Broadcast(CurrentChargeCount, GetMaxChargeCount());
}

int32 UETChargeAttackComponent::GetMaxChargeCount() const
{
	if (AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner()))
	{
		if (UETGameStatComponent* GameStatComponent = OwnerCharacter->GetGameStatComponent())
		{
			return FMath::Max(FMath::FloorToInt32(GameStatComponent->GetGameStat(EETGameStatType::HeavyAttackMaxCharge).GetMaxValue()), 0);
		}
	}

	return 0;
}

float UETChargeAttackComponent::GetDamageMultiplier() const
{
	return 1.f + CurrentChargeCount * ChargeDamageRate;
}

void UETChargeAttackComponent::OnChargeTimer()
{
	const int32 MaxChargeCount = GetMaxChargeCount();

	CurrentChargeCount = FMath::Min(CurrentChargeCount + 1, MaxChargeCount);
	OnChargeCountChanged.Broadcast(CurrentChargeCount, MaxChargeCount);

	if (CurrentChargeCount >= MaxChargeCount)
	{
		GetWorld()->GetTimerManager().ClearTimer(ChargeTimerHandle);
	}
}
