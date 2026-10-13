#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ETAnimNotifyState_PerfectTiming.generated.h"

// 회피, 패리 몽타주 시작 직후 구간 (이 구간에 적 공격이 닿으면 퍼펙트 판정)
UCLASS()
class ET_API UETAnimNotifyState_PerfectTiming : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
