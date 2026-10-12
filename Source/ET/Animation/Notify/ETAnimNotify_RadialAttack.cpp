#include "Animation/Notify/ETAnimNotify_RadialAttack.h"
#include "Character/ETCharacter.h"
#include "Components/ETAttackCollisionComponent.h"

void UETAnimNotify_RadialAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AETCharacter* OwnerCharacter = Cast<AETCharacter>(MeshComp->GetOwner()))
	{
		if (UETAttackCollisionComponent* AttackCollisionComponent = OwnerCharacter->GetAttackCollisionComponent())
		{
			AttackCollisionComponent->StartRadialAttackCollision(RadialAttackCollisionRowName);
		}
	}
}
