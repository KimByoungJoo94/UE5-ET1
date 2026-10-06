#include "Components/ETCombatComponent.h"
#include "Character/ETCharacter.h"
#include "Interface/ETCombatInterface.h"

UETCombatComponent::UETCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UETCombatComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UETCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (IsWeaponCollisionTracing())
    {
        TraceWeaponCollision();
    }
}

void UETCombatComponent::StartWeaponCollision()
{
    AlreadHitActorSet.Empty();
    bWeaponCollisionTracing = true;
}

void UETCombatComponent::EndWeaponCollision()
{
    bWeaponCollisionTracing = false;
}

void UETCombatComponent::TraceWeaponCollision()
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
                WeaponCollisionData.ActorsToIgnore,
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

                            if (IETCombatInterface* CombatInterface = Cast<IETCombatInterface>(HitActor))
                            {
                                CombatInterface->OnHit();                                
                            }
                        }
                    }
                }
            }
        }
    }
}