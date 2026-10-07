#include "Components/ETTimeFreezeComponent.h"
#include "TimerManager.h"

UETTimeFreezeComponent::UETTimeFreezeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UETTimeFreezeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UnfreezeTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UETTimeFreezeComponent::Freeze(const float InDuration)
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return;
	}

	if (bFrozen == false)
	{
		OriginCustomTimeDilation = OwnerActor->CustomTimeDilation;
		OwnerActor->CustomTimeDilation = 0.f;
		bFrozen = true;
	}

	// 액터 CustomTimeDilation 영향을 받지 않는 월드 타이머로 복구 (전체 TimeDilation 은 적용됨)
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer(UnfreezeTimerHandle);

	if (InDuration > 0.f)
	{
		TimerManager.SetTimer(UnfreezeTimerHandle, this, &ThisClass::Unfreeze, InDuration, false);
	}
}

void UETTimeFreezeComponent::Unfreeze()
{
	GetWorld()->GetTimerManager().ClearTimer(UnfreezeTimerHandle);

	if (bFrozen == false)
	{
		return;
	}

	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->CustomTimeDilation = OriginCustomTimeDilation;
	}

	bFrozen = false;
}
