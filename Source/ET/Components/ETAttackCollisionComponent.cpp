#include "Components/ETAttackCollisionComponent.h"
#include "Character/ETCharacter.h"
#include "Components/ETTimeFreezeComponent.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "TimerManager.h"

namespace
{
    // 한 틱에 나눠서 스윕하는 최대 횟수 (히치로 이동 거리가 커져도 비용 제한)
    constexpr int32 MaxWeaponCollisionStepCount = 16;

    // 루프 파티클 최대 재생 시간
    constexpr float LoopParticleLifeTime = 5.f;
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

void UETAttackCollisionComponent::StartRadialAttackCollision(const FName InRowName)
{
    AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner());
    const FETRadialAttackCollisionTableRowData* RowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETRadialAttackCollisionTableRowData>(InRowName);
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

    SpawnParticle(RowData->CenterParticle, Origin, RowData->CenterParticleScale);

    for (AActor* OverlapActor : OverlapActorArray)
    {
        const FVector TargetLocation = OverlapActor->GetActorLocation();
        const FHitResult HitResult(OverlapActor, nullptr, TargetLocation, (TargetLocation - Origin).GetSafeNormal());

        if (OwnerCharacter->AttackTarget(OverlapActor, HitResult) > 0.f)
        {
            SpawnParticle(RowData->HitParticle, TargetLocation, RowData->HitParticleScale);
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

    const FVector StartSocketLocation = SkeletalMeshComponent->GetSocketLocation(CachedWeaponCollisionRowData->StartSocketName);
    const FVector EndSocketLocation = SkeletalMeshComponent->GetSocketLocation(CachedWeaponCollisionRowData->EndSocketName);

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

                    if (OwnerCharacter->AttackTarget(HitActor, HitResult) > 0.f)
                    {
                        SpawnParticle(CachedWeaponCollisionRowData->HitParticle, HitResult.ImpactPoint, CachedWeaponCollisionRowData->HitParticleScale);
                    }
                }
            }
        }
    }
}

void UETAttackCollisionComponent::SpawnParticle(const TSoftObjectPtr<UParticleSystem>& InParticle, const FVector& InLocation, const float InScale)
{
    UParticleSystem* Particle = InParticle.LoadSynchronous();
    if (Particle == nullptr)
    {
        return;
    }

    UParticleSystemComponent* ParticleSystemComponent = UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), Particle, InLocation, FRotator::ZeroRotator, FVector(InScale));

    // 루프 파티클은 끝나지 않아 자동 제거되지 않으므로 일정 시간 뒤 정지 (남은 입자가 사라지면 자동 제거)
    if (ParticleSystemComponent && Particle->IsLooping())
    {
        FTimerHandle DeactivateTimerHandle;
        GetWorld()->GetTimerManager().SetTimer(
            DeactivateTimerHandle,
            FTimerDelegate::CreateWeakLambda(ParticleSystemComponent, [ParticleSystemComponent]()
            {
                ParticleSystemComponent->DeactivateSystem();
            }),
            LoopParticleLifeTime,
            false);
    }
}
