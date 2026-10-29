#include "Components/ETAttackCollisionComponent.h"
#include "Character/ETCharacter.h"
#include "Components/ETTimeFreezeComponent.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Subsystem/ETJobSubsystem.h"
#include "Job/ETCameraJobs.h"
#include "Common/ETHelper.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"

namespace
{
    // 한 틱에 나눠서 스윕하는 최대 횟수 (히치로 이동 거리가 커져도 비용 제한)
    constexpr int32 MaxWeaponCollisionStepCount = 16;
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

    // 시간 정지 중에는 무기가 멈춰 있어도 판정하지 않음 (해제 후 이어서 판정)
    AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner());
    if (OwnerCharacter && OwnerCharacter->GetTimeFreezeComponent()->IsFrozen())
    {
        return;
    }

    if (IsWeaponCollisionTracing())
    {
        TraceWeaponCollision();
    }
}

void UETAttackCollisionComponent::StartWeaponCollision(const FName InRowName)
{
    CachedWeaponCollisionRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETWeaponCollisionTableRowData>(InRowName);

    AlreadHitActorSet.Empty();
    bWeaponCollisionTracing = CachedWeaponCollisionRowData != nullptr;
    bHasLastLocation = false;
}

void UETAttackCollisionComponent::EndWeaponCollision()
{
    bWeaponCollisionTracing = false;
    CachedWeaponCollisionRowData = nullptr;
    bHasLastLocation = false;
}

void UETAttackCollisionComponent::StartRadialAttackCollision(const FName InRowName, const FName InSocketName)
{
    AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner());
    const FETRadialAttackCollisionTableRowData* RowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETRadialAttackCollisionTableRowData>(InRowName);
    if (OwnerCharacter == nullptr || RowData == nullptr || RowData->Radius <= 0.f)
    {
        return;
    }

    // 소켓이 유효하면 소켓 위치, 회전 기준 오프셋, 아니면 오너 기준 오프셋
    const USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter->GetMesh();
    const bool bUseSocket = InSocketName.IsNone() == false && SkeletalMeshComponent && SkeletalMeshComponent->DoesSocketExist(InSocketName);
    const FTransform OriginTransform = bUseSocket ? SkeletalMeshComponent->GetSocketTransform(InSocketName) : OwnerCharacter->GetActorTransform();
    const FVector Origin = OriginTransform.TransformPosition(RowData->LocationOffset);
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

    FETHelper::SpawnParticleAtLocation(this, RowData->CenterParticle, Origin, RowData->CenterParticleScale);
    PlayCameraShake(RowData->CameraShakeRowName, &Origin);

    for (AActor* OverlapActor : OverlapActorArray)
    {
        const FVector TargetLocation = OverlapActor->GetActorLocation();
        const FHitResult HitResult(OverlapActor, nullptr, TargetLocation, (TargetLocation - Origin).GetSafeNormal());

        if (OwnerCharacter->AttackTarget(OverlapActor, HitResult, OwnerCharacter->GetAttackDamage()) > 0.f)
        {
            FETHelper::SpawnParticleAtLocation(this, RowData->HitParticle, TargetLocation, RowData->HitParticleScale);
        }
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

void UETAttackCollisionComponent::PlayCameraShake(const FName InCameraShakeRowName, const FVector* InEpicenter)
{
    if (InCameraShakeRowName.IsNone())
    {
        return;
    }

    TUniquePtr<FETCameraShakeJob> CameraShakeJob = InEpicenter ? MakeUnique<FETCameraShakeJob>(InCameraShakeRowName, *InEpicenter) : MakeUnique<FETCameraShakeJob>(InCameraShakeRowName);
    ETSubsystem::Get<UETJobSubsystem>(this).Run(this, MoveTemp(CameraShakeJob));
}

void UETAttackCollisionComponent::TraceWeaponCollision()
{
    if (CachedWeaponCollisionRowData == nullptr)
    {
        return;
    }

    AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner());
    USkeletalMeshComponent* SkeletalMeshComponent = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
    if (SkeletalMeshComponent == nullptr)
    {
        return;
    }

    const FVector StartSocketLocation = SkeletalMeshComponent->GetSocketTransform(CachedWeaponCollisionRowData->StartSocketName).TransformPosition(CachedWeaponCollisionRowData->StartLocationOffset);
    const FVector EndSocketLocation = SkeletalMeshComponent->GetSocketTransform(CachedWeaponCollisionRowData->EndSocketName).TransformPosition(CachedWeaponCollisionRowData->EndLocationOffset);

    // 무기 축 방향으로 양 끝 길이 보정
    const FVector WeaponDirection = (EndSocketLocation - StartSocketLocation).GetSafeNormal();
    const FVector StartLocation = StartSocketLocation - WeaponDirection * CachedWeaponCollisionRowData->StartLengthOffset;
    const FVector EndLocation = EndSocketLocation + WeaponDirection * CachedWeaponCollisionRowData->EndLengthOffset;

    // 빠르게 휘두르면 틱 사이 궤적에 틈이 생기므로 이전 틱 위치부터 Radius 간격으로 나눠서 스윕
    int32 StepCount = 1;
    if (bHasLastLocation)
    {
        const float MoveDistance = FMath::Max(FVector::Dist(LastStartLocation, StartLocation), FVector::Dist(LastEndLocation, EndLocation));
        const float StepLength = FMath::Max(CachedWeaponCollisionRowData->Radius, 1.f);
        StepCount = FMath::Clamp(FMath::CeilToInt32(MoveDistance / StepLength), 1, MaxWeaponCollisionStepCount);
    }

    for (int32 StepIndex = 1; StepIndex <= StepCount; ++StepIndex)
    {
        const float Alpha = bHasLastLocation ? static_cast<float>(StepIndex) / StepCount : 1.f;
        SweepWeaponCollision(
            OwnerCharacter,
            FMath::Lerp(LastStartLocation, StartLocation, Alpha),
            FMath::Lerp(LastEndLocation, EndLocation, Alpha));
    }

    LastStartLocation = StartLocation;
    LastEndLocation = EndLocation;
    bHasLastLocation = true;
}

void UETAttackCollisionComponent::SweepWeaponCollision(AETCharacter* OwnerCharacter, const FVector& StartLocation, const FVector& EndLocation)
{
    TArray<FHitResult> HitResultArray;

    bool const bHit = UKismetSystemLibrary::SphereTraceMultiForObjects(
        OwnerCharacter,
        StartLocation,
        EndLocation,
        CachedWeaponCollisionRowData->Radius,
        CachedWeaponCollisionRowData->ObjectTypeArray,
        false,
        TArray<AActor*>(),
        CachedWeaponCollisionRowData->DrawDebugType,
        HitResultArray,
        true,
        FLinearColor::Red,
        FLinearColor::Green,
        CachedWeaponCollisionRowData->DrawDebugTime);

    if (bHit)
    {
        for (const FHitResult& HitResult : HitResultArray)
        {
            if (AActor* HitActor = HitResult.GetActor())
            {
                if (AlreadHitActorSet.Contains(HitActor) == false)
                {
                    AlreadHitActorSet.Add(HitActor);

                    if (OwnerCharacter->AttackTarget(HitActor, HitResult, OwnerCharacter->GetAttackDamage()) > 0.f)
                    {
                        FETHelper::SpawnParticleAtLocation(this, CachedWeaponCollisionRowData->HitParticle, HitResult.ImpactPoint, CachedWeaponCollisionRowData->HitParticleScale);
                        PlayCameraShake(CachedWeaponCollisionRowData->CameraShakeRowName);
                    }
                }
            }
        }
    }
}
