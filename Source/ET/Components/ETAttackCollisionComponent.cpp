#include "Components/ETAttackCollisionComponent.h"
#include "Character/ETCharacter.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

namespace
{
	template <typename T>
	const T* FindCollisionRowData(const UObject* InWorldContextObject, const FName InRowName)
	{
		if (const UETGameDataSubsystem* GameDataSubsystem = UGameInstance::GetSubsystem<UETGameDataSubsystem>(UGameplayStatics::GetGameInstance(InWorldContextObject)))
		{
			return GameDataSubsystem->GetRow<T>(InRowName);
		}

		return nullptr;
	}
}

UETAttackCollisionComponent::UETAttackCollisionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UETAttackCollisionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UETAttackCollisionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (IsWeaponCollisionTracing())
    {
        TraceWeaponCollision();
    }
}

void UETAttackCollisionComponent::StartWeaponCollision(const FName InRowName)
{
    WeaponCollisionRowData = FindCollisionRowData<FETWeaponCollisionTableRowData>(this, InRowName);

    AlreadHitActorSet.Empty();
    bWeaponCollisionTracing = WeaponCollisionRowData != nullptr;
}

void UETAttackCollisionComponent::EndWeaponCollision()
{
    bWeaponCollisionTracing = false;
    WeaponCollisionRowData = nullptr;
}

void UETAttackCollisionComponent::StartRadialAttackCollision(const FName InRowName)
{
    AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner());
    const FETRadialAttackCollisionTableRowData* RowData = FindCollisionRowData<FETRadialAttackCollisionTableRowData>(this, InRowName);
    if (OwnerCharacter == nullptr || RowData == nullptr || RowData->Radius <= 0.f)
    {
        return;
    }

    const FVector Origin = OwnerCharacter->GetActorTransform().TransformPosition(RowData->LocationOffset);
    const TArray<AActor*> IgnoreActorArray = { OwnerCharacter };
    TArray<AActor*> OverlapActorArray;

    UKismetSystemLibrary::SphereOverlapActors(
        OwnerCharacter,
        Origin,
        RowData->Radius,
        RowData->ObjectTypeArray,
        AETCharacter::StaticClass(),
        IgnoreActorArray,
        OverlapActorArray);

    for (AActor* OverlapActor : OverlapActorArray)
    {
        const FVector TargetLocation = OverlapActor->GetActorLocation();
        const FHitResult HitResult(OverlapActor, nullptr, TargetLocation, (TargetLocation - Origin).GetSafeNormal());

        OwnerCharacter->AttackTarget(OverlapActor, HitResult);
    }

#if ENABLE_DRAW_DEBUG
    if (RowData->DrawDebugType != EDrawDebugTrace::None)
    {
        const bool bPersistent = RowData->DrawDebugType == EDrawDebugTrace::Persistent;
        const float LifeTime = RowData->DrawDebugType == EDrawDebugTrace::ForDuration ? RowData->DrawDebugTime : 0.f;
        const FColor DebugColor = OverlapActorArray.IsEmpty() ? FColor::Red : FColor::Green;

        DrawDebugSphere(GetWorld(), Origin, RowData->Radius, 24, DebugColor, bPersistent, LifeTime);
    }
#endif
}

void UETAttackCollisionComponent::TraceWeaponCollision()
{
    if (WeaponCollisionRowData == nullptr)
    {
        return;
    }

    if (AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner()))
    {
        if (USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh())
        {
            const FVector& StartLocation = SkeletalMeshComponent->GetSocketLocation(WeaponCollisionRowData->StartSocketName);
            const FVector& EndLocation = SkeletalMeshComponent->GetSocketLocation(WeaponCollisionRowData->EndSocketName);
            TArray<FHitResult> HitResultArray;

            bool const bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
                OwnerCharacter,
                StartLocation,
                EndLocation,
                WeaponCollisionRowData->Radius,
                WeaponCollisionRowData->ObjectTypeArray,
                false,
                TArray<AActor*>(),
                WeaponCollisionRowData->DrawDebugType,
                HitResultArray,
                true,
                FLinearColor::Red,
                FLinearColor::Green,
                WeaponCollisionRowData->DrawDebugTime);

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
