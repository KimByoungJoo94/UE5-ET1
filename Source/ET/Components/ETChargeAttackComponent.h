#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TimerHandle.h"
#include "ETChargeAttackComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_TwoParams(FOnChargeCountChanged, const int32 InCurrentChargeCount, const int32 InMaxChargeCount);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETChargeAttackComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UETChargeAttackComponent();

public:
	void StartChargeAttack();
	void EndChargeAttack();
	void ResetChargeAttack();

	FORCEINLINE int32 GetCurrentChargeCount() const { return CurrentChargeCount; }
	int32 GetMaxChargeCount() const;
	float GetDamageMultiplier() const;

protected:
	void OnChargeTimer();

public:
	FOnChargeCountChanged OnChargeCountChanged;

protected:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.01"))
	float ChargeInterval = 1.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float ChargeDamageRate = 0.5f;

private:
	int32 CurrentChargeCount = 0;
	FTimerHandle ChargeTimerHandle;
};
