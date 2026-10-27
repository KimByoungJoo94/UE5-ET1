#include "Animation/Notify/ETAnimNotify_FireProjectile.h"
#include "Character/ETCharacter.h"
#include "Projectile/ETProjectile.h"
#include "Components/SkeletalMeshComponent.h"

void UETAnimNotify_FireProjectile::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// 에디터 프리뷰 등 게임 월드의 캐릭터가 아니면 발사하지 않음
	AETCharacter* OwnerCharacter = MeshComp ? Cast<AETCharacter>(MeshComp->GetOwner()) : nullptr;
	UWorld* World = OwnerCharacter ? OwnerCharacter->GetWorld() : nullptr;
	if (World == nullptr || World->IsGameWorld() == false)
	{
		return;
	}

	const FVector StartLocation = MeshComp->GetSocketTransform(StartSocketName).TransformPosition(StartLocationOffset);
	const FVector EndLocation = MeshComp->GetSocketTransform(EndSocketName).TransformPosition(EndLocationOffset);

	// 두 위치가 같으면 방향을 알 수 없으므로 캐릭터 정면
	const FVector FireDirection = (EndLocation - StartLocation).GetSafeNormal();
	const FRotator FireRotation = FireDirection.IsNearlyZero() ? OwnerCharacter->GetActorRotation() : FireDirection.Rotation();
	const FTransform SpawnTransform(FireRotation, EndLocation);

	// PreInitializeComponents 에서 RowName 으로 데이터를 적용하므로 FinishSpawning 전에 지정
	AETProjectile* Projectile = World->SpawnActorDeferred<AETProjectile>(AETProjectile::StaticClass(), SpawnTransform, OwnerCharacter, OwnerCharacter, ESpawnActorCollisionHandlingMethod::AlwaysSpawn);
	if (Projectile == nullptr)
	{
		return;
	}

	Projectile->SetProjectileRowName(ProjectileRowName);
	Projectile->FinishSpawning(SpawnTransform);
}
