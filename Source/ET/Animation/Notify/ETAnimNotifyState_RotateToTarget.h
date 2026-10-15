#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ETAnimNotifyState_RotateToTarget.generated.h"

class AETCharacter;

UENUM()
enum class EETRotateTargetType : uint8
{
	AttackTarget,
	InteractionTarget,
};

// 구간 동안 지정한 종류의 대상 방향으로 일정 속도 회전 (공격 예비 동작에서 추적, 타격 구간 전에 끝내서 회피 가능하게)
UCLASS()
class ET_API UETAnimNotifyState_RotateToTarget : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;

private:
	AActor* GetTarget(const AETCharacter* InOwnerCharacter) const;

protected:
	UPROPERTY(EditAnywhere)
	EETRotateTargetType TargetType = EETRotateTargetType::AttackTarget;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float RotationSpeed = 360.f;
};
