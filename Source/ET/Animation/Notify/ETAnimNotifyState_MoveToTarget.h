#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotifyState.h"
#include "ETAnimNotifyState_MoveToTarget.generated.h"

// 구간 동안 공격 대상 쪽으로 걷기 속도로 이동 (제자리 연속 공격 추격용, 몽타주 FullBody 커브를 꺼서 하체는 로코모션)
// 멈추는 거리는 현재 행동 액션 조건의 MaxRange (캡슐 사이 거리, BT 접근 거리와 동일), 액션 조건이 없으면 이동 안 함
UCLASS()
class ET_API UETAnimNotifyState_MoveToTarget : public UAnimNotifyState
{
	GENERATED_BODY()

public:
	virtual void NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference) override;
};
