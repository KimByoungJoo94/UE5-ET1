#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ETAnimNotify_Defeat.generated.h"

// 쓰러짐 연출이 끝난 시점 (사망 몽타주에서 사용)
UCLASS()
class ET_API UETAnimNotify_Defeat : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;
};
