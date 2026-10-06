#include "Animation/Notify/ETAnimNotify_PauseHeavyAttack.h"
#include "Interface/ETCombatInterface.h"

void UETAnimNotify_PauseHeavyAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (IETCombatInterface* CombatInterface = Cast<IETCombatInterface>(OwnerActor))
		{
			CombatInterface->OnHeavyAttackPause();
		}
	}
}