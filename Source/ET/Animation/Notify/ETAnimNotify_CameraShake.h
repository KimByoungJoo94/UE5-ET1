#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ETAnimNotify_CameraShake.generated.h"

// 공격 판정 없이 연출용 카메라 쉐이크 (포효, 착지 등)
// 플레이어가 재생하면 그대로, 몬스터 등 다른 캐릭터가 재생하면 오너 위치를 진원지로 테이블 Inner, Outer 반경 감쇠
UCLASS()
class ET_API UETAnimNotify_CameraShake : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetCameraShakeRowNameOptions"))
	FName CameraShakeRowName;
};
