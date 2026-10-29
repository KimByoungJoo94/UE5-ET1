#include "Common/ETHelper.h"
#include "DataAsset/ETCharacterTableRowData.h"
#include "Engine/World.h"
#include "GameFramework/Actor.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"

namespace
{
	// 루프 파티클 최대 재생 시간
	constexpr float LoopParticleLifeTime = 5.f;
}

void FETHelper::SpawnParticleAtLocation(const UObject* InWorldContextObject, const TSoftObjectPtr<UParticleSystem>& InParticle, const FVector& InLocation, const FRotator& InRotation, const float InScale)
{
	UParticleSystem* Particle = InParticle.LoadSynchronous();
	UWorld* World = InWorldContextObject ? InWorldContextObject->GetWorld() : nullptr;
	if (Particle == nullptr || World == nullptr)
	{
		return;
	}

	UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(World, Particle, InLocation, InRotation, FVector(InScale));

	// 루프 파티클은 끝나지 않아 자동 제거되지 않으므로 일정 시간 뒤 정지 (남은 입자가 사라지면 자동 제거)
	if (ParticleSystemComponent && Particle->IsLooping())
	{
		FTimerHandle DeactivateTimerHandle;
		World->GetTimerManager().SetTimer(
			DeactivateTimerHandle,
			FTimerDelegate::CreateWeakLambda(ParticleSystemComponent, [ParticleSystemComponent]()
			{
				ParticleSystemComponent->DeactivateSystem();
			}),
			LoopParticleLifeTime,
			false);
	}
}

UParticleSystemComponent* FETHelper::SpawnParticleAttached(const TSoftObjectPtr<UParticleSystem>& InParticle, USceneComponent* InAttachComponent, const FName InSocketName)
{
	UParticleSystem* Particle = InParticle.LoadSynchronous();
	if (Particle == nullptr || InAttachComponent == nullptr)
	{
		return nullptr;
	}

	return UGameplayStatics::SpawnEmitterAttached(Particle, InAttachComponent, InSocketName);
}

bool FETHelper::IsHostile(const EETCharacterType InAttackerType, const EETCharacterType InTargetType)
{
	return (InAttackerType == EETCharacterType::Player) != (InTargetType == EETCharacterType::Player);
}

bool FETHelper::IsInSight(const AActor* InOwner, const AActor* InTarget, const float InSightAngle)
{
	const FVector ForwardDirection = InOwner->GetActorForwardVector().GetSafeNormal2D();
	const FVector TargetDirection = (InTarget->GetActorLocation() - InOwner->GetActorLocation()).GetSafeNormal2D();
	if (FVector::DotProduct(ForwardDirection, TargetDirection) < FMath::Cos(FMath::DegreesToRadians(InSightAngle * 0.5f)))
	{
		return false;
	}

	FVector EyeLocation;
	FRotator EyeRotation;
	InOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ETHelperIsInSight), false, InOwner);
	return InOwner->GetWorld()->LineTraceTestByObjectType(EyeLocation, InTarget->GetActorLocation(), FCollisionObjectQueryParams(ECC_WorldStatic), QueryParams) == false;
}

float FETHelper::GetCollisionDistance2D(const AActor* InActorA, const AActor* InActorB)
{
	float RadiusA = 0.f;
	float HalfHeightA = 0.f;
	float RadiusB = 0.f;
	float HalfHeightB = 0.f;
	InActorA->GetSimpleCollisionCylinder(RadiusA, HalfHeightA);
	InActorB->GetSimpleCollisionCylinder(RadiusB, HalfHeightB);

	return FMath::Max(FVector::Dist2D(InActorA->GetActorLocation(), InActorB->GetActorLocation()) - RadiusA - RadiusB, 0.f);
}
