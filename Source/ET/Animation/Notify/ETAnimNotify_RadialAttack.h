#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ETAnimNotify_RadialAttack.generated.h"

// 노티파이 시점에 오너 기준 원형 범위 안의 모든 캐릭터 공격
UCLASS()
class ET_API UETAnimNotify_RadialAttack : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetRadialAttackCollisionRowNameOptions"))
	FName RadialAttackCollisionRowName;
};
