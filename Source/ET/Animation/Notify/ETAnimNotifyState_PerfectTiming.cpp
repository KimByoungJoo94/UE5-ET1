#include "Animation/Notify/ETAnimNotifyState_PerfectTiming.h"
#include "Character/ETPlayer.h"

void UETAnimNotifyState_PerfectTiming::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AETPlayer* OwnerPlayer = Cast<AETPlayer>(MeshComp->GetOwner()))
	{
		OwnerPlayer->SetPerfectTiming(true);
	}
}

void UETAnimNotifyState_PerfectTiming::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	// 몽타주가 끊겨도 호출되므로 구간이 남지 않음
	if (AETPlayer* OwnerPlayer = Cast<AETPlayer>(MeshComp->GetOwner()))
	{
		OwnerPlayer->SetPerfectTiming(false);
	}
}
