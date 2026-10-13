#include "Animation/Notify/ETAnimNotifyState_RotateToTarget.h"
#include "Character/ETCharacter.h"

void UETAnimNotifyState_RotateToTarget::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	AETCharacter* OwnerCharacter = Cast<AETCharacter>(MeshComp->GetOwner());
	const AActor* TargetActor = OwnerCharacter ? OwnerCharacter->GetAttackTarget() : nullptr;
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
