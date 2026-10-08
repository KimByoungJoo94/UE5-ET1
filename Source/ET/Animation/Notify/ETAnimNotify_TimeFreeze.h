#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ETAnimNotify_TimeFreeze.generated.h"

// 시간 정지 시전 시점
UCLASS()
class ET_API UETAnimNotify_TimeFreeze : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetTimeFreezeRowNameOptions"))
	FName TimeFreezeRowName;
};
