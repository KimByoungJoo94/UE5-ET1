#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TimerHandle.h"
#include "ETTimeFreezeComponent.generated.h"

class AETCharacter;
struct FETTimeFreezeTableRowData;

// 시간 정지 시전 (주변 대상 탐색 후 정지) 및 Owner 자신의 정지 / 해제
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETTimeFreezeComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UETTimeFreezeComponent();

protected:
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	void CastTimeFreeze(const FName InRowName);
	void Freeze(const float InDuration);
	void Unfreeze();

	FORCEINLINE bool IsFrozen() const { return bFrozen; }

protected:
	void FindTimeFreezeTargets(const FETTimeFreezeTableRowData& InRowData, OUT TArray<AETCharacter*>& OutTargetArray) const;

private:
	bool bFrozen = false;
	float OriginCustomTimeDilation = 1.f;
	FTimerHandle UnfreezeTimerHandle;
};
