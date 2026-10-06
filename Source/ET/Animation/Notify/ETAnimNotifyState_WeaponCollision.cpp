#include "Animation/Notify/ETAnimNotifyState_WeaponCollision.h"
#include "Interface/ETCombatInterface.h"

void UETAnimNotifyState_WeaponCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (IETCombatInterface* CombatInterface = Cast<IETCombatInterface>(OwnerActor))
		{
			CombatInterface->OnStartWeaponCollision();
		}
	}
}

void UETAnimNotifyState_WeaponCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (IETCombatInterface* CombatInterface = Cast<IETCombatInterface>(OwnerActor))
		{
			CombatInterface->OnEndWeaponCollision();
		}
	}
}
