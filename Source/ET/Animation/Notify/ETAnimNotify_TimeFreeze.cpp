#include "Animation/Notify/ETAnimNotify_TimeFreeze.h"
#include "Character/ETCharacter.h"
#include "Components/ETTimeFreezeComponent.h"

void UETAnimNotify_TimeFreeze::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AETCharacter* OwnerCharacter = Cast<AETCharacter>(MeshComp->GetOwner()))
	{
		if (UETTimeFreezeComponent* TimeFreezeComponent = OwnerCharacter->GetTimeFreezeComponent())
		{
			TimeFreezeComponent->CastTimeFreeze(TimeFreezeRowName);
		}
	}
}
