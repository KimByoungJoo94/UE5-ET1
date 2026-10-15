#include "Animation/Notify/ETAnimNotifyState_RotateToTarget.h"
#include "Character/ETCharacter.h"
#include "Components/ETInteractionComponent.h"

void UETAnimNotifyState_RotateToTarget::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	AETCharacter* OwnerCharacter = Cast<AETCharacter>(MeshComp->GetOwner());
	const AActor* TargetActor = OwnerCharacter ? GetTarget(OwnerCharacter) : nullptr;
	if (TargetActor == nullptr)
	{
		return;
	}

	const FVector ToTarget = (TargetActor->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
	if (ToTarget.IsNearlyZero())
	{
		return;
	}

	FRotator NewRotation = OwnerCharacter->GetActorRotation();
	NewRotation.Yaw = FMath::FixedTurn(NewRotation.Yaw, ToTarget.Rotation().Yaw, RotationSpeed * FrameDeltaTime);
	OwnerCharacter->SetActorRotation(NewRotation);
}

AActor* UETAnimNotifyState_RotateToTarget::GetTarget(const AETCharacter* InOwnerCharacter) const
{
	switch (TargetType)
	{
	case EETRotateTargetType::AttackTarget:
		return InOwnerCharacter->GetAttackTarget();

	case EETRotateTargetType::InteractionTarget:
		{
			const UETInteractionComponent* InteractionComponent = InOwnerCharacter->FindComponentByClass<UETInteractionComponent>();
			return InteractionComponent ? InteractionComponent->GetInteractionTarget() : nullptr;
		}
	}

	return nullptr;
}
