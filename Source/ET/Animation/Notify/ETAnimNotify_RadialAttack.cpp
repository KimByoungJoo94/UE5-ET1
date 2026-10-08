#include "Animation/Notify/ETAnimNotify_RadialAttack.h"
#include "Components/ETAttackCollisionComponent.h"

void UETAnimNotify_RadialAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UETAttackCollisionComponent* AttackCollisionComponent = OwnerActor->FindComponentByClass<UETAttackCollisionComponent>())
		{
			AttackCollisionComponent->StartRadialAttackCollision(RadialAttackCollisionRowName);
		}
	}
}
