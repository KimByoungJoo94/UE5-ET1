#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ETAnimNotify_Dissolve.generated.h"

// 디졸브 시작 시점 (사망 몽타주에서 사용)
UCLASS()
class ET_API UETAnimNotify_Dissolve : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
