#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "AlphaBlend.h"
#include "ETAnimNotifyState_CapsuleHalfHeight.generated.h"

UCLASS()
class ET_API UETAnimNotifyState_CapsuleHalfHeight : public UAnimNotifyState
{
	GENERATED_BODY()
	
public:
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float TargetHalfHeight = 40.f;

	// 구간 시작부터 이 비율까지 기본 높이 -> 목표 높이
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float ShrinkRatio = 0.3f;

	// 구간 끝에서 이 비율만큼 목표 높이 -> 기본 높이
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float RestoreRatio = 0.3f;

	UPROPERTY(EditAnywhere)
	EAlphaBlendOption BlendOption = EAlphaBlendOption::CubicInOut;
};
