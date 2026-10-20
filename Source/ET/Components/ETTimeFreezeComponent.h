#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Engine/TimerHandle.h"
#include "ETTimeFreezeComponent.generated.h"

class AETCharacter;
class UMaterialInterface;
class UFXSystemComponent;
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

	// 정지 동안 Owner 캐릭터 메시의 모든 슬롯을 정지 머터리얼로 교체 / 원래 머터리얼로 복구
	void ApplyTimeFreezeMaterial();
	void RestoreOriginMaterials();

	// 정지 동안 메시에 붙은 런타임 생성 이펙트(몽타주 노티파이 트레일 등)를 숨김 / 복구 (BP 에 배치한 이펙트는 유지)
	void HideSpawnedEffects();
	void RestoreSpawnedEffects();

private:
	// 정지 전 메시 슬롯별 머터리얼
	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInterface>> OriginMaterialArray;

	// 정지하며 숨긴 이펙트 (정지 중 자동 파괴될 수 있으므로 약참조)
	TArray<TWeakObjectPtr<UFXSystemComponent>> HiddenEffectArray;

	bool bFrozen = false;
	FTimerHandle UnfreezeTimerHandle;
};
