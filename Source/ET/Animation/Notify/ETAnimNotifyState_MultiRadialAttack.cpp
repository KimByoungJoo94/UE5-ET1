#include "Animation/Notify/ETAnimNotifyState_MultiRadialAttack.h"
#include "Character/ETCharacter.h"

void UETAnimNotifyState_MultiRadialAttack::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AETCharacter* OwnerCharacter = Cast<AETCharacter>(MeshComp->GetOwner()))
	{
		if (UETAttackCollisionComponent* AttackCollisionComponent = OwnerCharacter->GetAttackCollisionComponent())
		{
			AttackCollisionComponent->StartMultiRadialAttackCollision(MultiRadialAttackData, TotalDuration);
		}
	}
}

void UETAnimNotifyState_MultiRadialAttack::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AETCharacter* OwnerCharacter = Cast<AETCharacter>(MeshComp->GetOwner()))
	{
		if (UETAttackCollisionComponent* AttackCollisionComponent = OwnerCharacter->GetAttackCollisionComponent())
		{
			AttackCollisionComponent->EndMultiRadialAttackCollision();
		}
	}
}
