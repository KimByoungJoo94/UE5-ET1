#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ETAnimNotifyState_RotateToTarget.generated.h"

// 구간 동안 공격 대상(GetAttackTarget) 방향으로 일정 속도 회전 (예비 동작에서 추적, 타격 구간 전에 끝내서 회피 가능하게)
UCLASS()
class ET_API UETAnimNotifyState_RotateToTarget : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float RotationSpeed = 360.f;
};
