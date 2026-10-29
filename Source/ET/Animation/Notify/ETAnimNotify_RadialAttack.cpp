#include "Animation/Notify/ETAnimNotify_RadialAttack.h"
#include "Character/ETCharacter.h"
#include "Components/ETAttackCollisionComponent.h"
#include "Common/ETHelper.h"
#include "DataAsset/ETCollisionTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"

void UETAnimNotify_RadialAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AETCharacter* OwnerCharacter = Cast<AETCharacter>(MeshComp->GetOwner()))
	{
		if (UETAttackCollisionComponent* AttackCollisionComponent = OwnerCharacter->GetAttackCollisionComponent())
		{
			AttackCollisionComponent->StartRadialAttackCollision(RadialAttackCollisionRowName, SocketName, LocationOffset, RotationOffset);
		}
	}
#if WITH_EDITOR
	// 몽타주 에디터 미리보기 (캐릭터가 없으므로 판정 없이 중심 파티클만, 소켓이 없으면 메시 기준)
	else if (MeshComp->GetWorld() && MeshComp->GetWorld()->IsGameWorld() == false)
	{
		if (const FETRadialAttackCollisionTableRowData* RowData = UETGameDataSubsystem::GetRowInEditor<FETRadialAttackCollisionTableRowData>(RadialAttackCollisionRowName))
		{
			const bool bUseSocket = SocketName.IsNone() == false && MeshComp->DoesSocketExist(SocketName);
			const FTransform OriginTransform = bUseSocket ? MeshComp->GetSocketTransform(SocketName) : MeshComp->GetComponentTransform();
			const FRotator CenterParticleRotation = (OriginTransform.GetRotation() * FQuat(RotationOffset)).Rotator();
			FETHelper::SpawnParticleAtLocation(MeshComp, RowData->CenterParticle, OriginTransform.TransformPosition(LocationOffset), CenterParticleRotation, RowData->CenterParticleScale);
		}
	}
#endif
}
