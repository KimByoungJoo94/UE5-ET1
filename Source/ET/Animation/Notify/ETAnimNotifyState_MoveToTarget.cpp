#include "Animation/Notify/ETAnimNotifyState_MoveToTarget.h"
#include "Character/ETCharacter.h"
#include "DataAsset/ETActionConditionTableRowData.h"

void UETAnimNotifyState_MoveToTarget::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	AETCharacter* OwnerCharacter = Cast<AETCharacter>(MeshComp->GetOwner());
	const AActor* TargetActor = OwnerCharacter ? OwnerCharacter->GetAttackTarget() : nullptr;
	const FETActionConditionTableRowData* ConditionRowData = OwnerCharacter ? OwnerCharacter->GetCurrentActionConditionRowData() : nullptr;
	if (TargetActor == nullptr || ConditionRowData == nullptr)
	{
		return;
	}

	const FVector ToTarget = (TargetActor->GetActorLocation() - OwnerCharacter->GetActorLocation()) * FVector(1.f, 1.f, 0.f);
	const float Gap = ToTarget.Size() - OwnerCharacter->GetSimpleCollisionRadius() - TargetActor->GetSimpleCollisionRadius();
	if (Gap <= ConditionRowData->MaxRange)
	{
		return;
	}

	// 이동 컴포넌트를 거쳐 충돌, 속도, 하체 애니메이션(Speed)이 그대로 적용되도록 입력으로 이동
	OwnerCharacter->AddMovementInput(ToTarget.GetSafeNormal());
}
