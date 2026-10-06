#include "Components/ETWeaponCollisionComponent.h"
#include "Character/ETCharacter.h"

UETWeaponCollisionComponent::UETWeaponCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UETWeaponCollisionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UETWeaponCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (IsWeaponCollisionTracing())
    {
        TraceWeaponCollision();
    }
}

void UETWeaponCollisionComponent::StartWeaponCollision()
{
    AlreadHitActorSet.Empty();
    bWeaponCollisionTracing = true;
}

void UETWeaponCollisionComponent::EndWeaponCollision()
{
    bWeaponCollisionTracing = false;
}

void UETWeaponCollisionComponent::TraceWeaponCollision()
{
    if (AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh())
        {
            const FVector& StartLocation = SkeletalMeshComponent->GetSocketLocation(WeaponCollisionData.StartSocketName);
            const FVector& EndLocation = SkeletalMeshComponent->GetSocketLocation(WeaponCollisionData.EndSocketName);
            TArray<FHitResult> HitResultArray;

            bool const bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
                OwnerCharacter,
                StartLocation,
                EndLocation,
                WeaponCollisionData.Radius,
                WeaponCollisionData.ObjectTypes,
                false,
                TArray<AActor*>(),
                WeaponCollisionData.DrawDebugType,
                HitResultArray,
                true);

            if (bHit)
            {
                for (const FHitResult& HitResult : HitResultArray)
                {
                    if (AActor* HitActor = HitResult.GetActor())
                    {
                        if (AlreadHitActorSet.Contains(HitActor) == false)
                        {
                            AlreadHitActorSet.Add(HitActor);

                            OwnerCharacter->AttackTarget(HitActor, HitResult);
                        }
                    }
                }
            }
        }
    }
}
