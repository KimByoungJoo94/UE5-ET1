#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "Components/ETAttackCollisionComponent.h"
#include "ETAnimNotifyState_MultiRadialAttack.generated.h"

// 구간 동안 영역 안 랜덤 위치에 작은 범위 공격을 여러 번 (구간 길이를 웨이브 수로 나눈 간격, 웨이브마다 여러 개)
UCLASS()
class ET_API UETAnimNotifyState_MultiRadialAttack : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference) override;
	virtual void NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, meta = (ShowOnlyInnerProperties))
	FETMultiRadialAttackData MultiRadialAttackData;
};
