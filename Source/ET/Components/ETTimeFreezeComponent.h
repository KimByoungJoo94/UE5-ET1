#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TimerHandle.h"
#include "ETTimeFreezeComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETTimeFreezeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UETTimeFreezeComponent();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// InDuration 이 0 이하이면 Unfreeze 호출 전까지 정지 유지, 정지 중 호출 시 시간 갱신
	void Freeze(const float InDuration);
	void Unfreeze();

	FORCEINLINE bool IsFrozen() const { return bFrozen; }

private:
	bool bFrozen = false;
	float OriginCustomTimeDilation = 1.f;
	FTimerHandle UnfreezeTimerHandle;
};
