#include "Animation/Notify/ETAnimNotifyState_WeaponCollision.h"
#include "Components/ETAttackCollisionComponent.h"

void UETAnimNotifyState_WeaponCollision::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UETAttackCollisionComponent* AttackCollisionComponent = OwnerActor->FindComponentByClass<UETAttackCollisionComponent>())
		{
			AttackCollisionComponent->StartWeaponCollision(WeaponCollisionRowName);
		}
	}
}

void UETAnimNotifyState_WeaponCollision::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UETAttackCollisionComponent* AttackCollisionComponent = OwnerActor->FindComponentByClass<UETAttackCollisionComponent>())
		{
			AttackCollisionComponent->EndWeaponCollision();
		}
	}
}
