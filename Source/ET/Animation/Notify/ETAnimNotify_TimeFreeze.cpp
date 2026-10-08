#include "Animation/Notify/ETAnimNotify_TimeFreeze.h"
#include "Components/ETTimeFreezeComponent.h"

void UETAnimNotify_TimeFreeze::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UETTimeFreezeComponent* TimeFreezeComponent = OwnerActor->FindComponentByClass<UETTimeFreezeComponent>())
		{
			TimeFreezeComponent->CastTimeFreeze(TimeFreezeRowName);
		}
	}
}
