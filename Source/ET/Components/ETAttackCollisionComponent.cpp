#include "Components/ETAttackCollisionComponent.h"
#include "Character/ETCharacter.h"
#include "Components/ETTimeFreezeComponent.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Subsystem/ETJobSubsystem.h"
#include "Job/ETCameraJobs.h"
#include "Common/ETHelper.h"
#include "DrawDebugHelpers.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Controller.h"

namespace
{
    // 한 틱에 나눠서 스윕하는 최대 횟수 (히치로 이동 거리가 커져도 비용 제한)
    constexpr int32 MaxWeaponCollisionStepCount = 16;

    // 여러 번 범위 공격 위치를 바닥에 맞출 때 위아래 트레이스 거리
    constexpr float GroundTraceUpDistance = 300.f;
    constexpr float GroundTraceDownDistance = 1000.f;
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

    if (RemainingMultiRadialAttackWaveCount > 0)
    {
        MultiRadialAttackElapsedTime += DeltaTime;
        if (MultiRadialAttackElapsedTime >= MultiRadialAttackInterval)
        {
            MultiRadialAttackElapsedTime -= MultiRadialAttackInterval;
            PlayMultiRadialAttackWave();
        }
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

void UETAttackCollisionComponent::StartRadialAttackCollision(const FName InRowName, const FName InSocketName, const FVector& InLocationOffset, const FRotator& InRotationOffset)
{
    AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner());
    const FETRadialAttackCollisionTableRowData* RowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETRadialAttackCollisionTableRowData>(InRowName);
    if (OwnerCharacter == nullptr || RowData == nullptr || RowData->Radius <= 0.f)
    {
        return;
    }

    // 엔진 Play Particle Effect 노티파이와 같은 회전 (기준 회전 * 회전 오프셋)
    const FTransform OriginTransform = GetRadialAttackOriginTransform(OwnerCharacter, InSocketName);
    const FRotator CenterParticleRotation = (OriginTransform.GetRotation() * FQuat(InRotationOffset)).Rotator();
    PlayRadialAttack(OwnerCharacter, *RowData, OriginTransform.TransformPosition(InLocationOffset), CenterParticleRotation);
}

void UETAttackCollisionComponent::StartMultiRadialAttackCollision(const FETMultiRadialAttackData& InMultiRadialAttackData, const float InDuration)
{
    const FETRadialAttackCollisionTableRowData* RowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETRadialAttackCollisionTableRowData>(InMultiRadialAttackData.RadialAttackCollisionRowName);
    if (RowData == nullptr || RowData->Radius <= 0.f)
    {
        return;
    }

    MultiRadialAttackData = InMultiRadialAttackData;
    CachedMultiRadialAttackRowData = RowData;
    RemainingMultiRadialAttackWaveCount = FMath::Max(MultiRadialAttackData.WaveCount, 1);
    MultiRadialAttackInterval = InDuration / RemainingMultiRadialAttackWaveCount;
    MultiRadialAttackElapsedTime = 0.f;

    PlayMultiRadialAttackWave();
}

void UETAttackCollisionComponent::EndMultiRadialAttackCollision()
{
    RemainingMultiRadialAttackWaveCount = 0;
    CachedMultiRadialAttackRowData = nullptr;
}

FTransform UETAttackCollisionComponent::GetRadialAttackOriginTransform(const AETCharacter* InOwnerCharacter, const FName InSocketName) const
{
    const USkeletalMeshComponent* SkeletalMeshComponent = InOwnerCharacter->GetMesh();
    const bool bUseSocket = InSocketName.IsNone() == false && SkeletalMeshComponent && SkeletalMeshComponent->DoesSocketExist(InSocketName);
    return bUseSocket ? SkeletalMeshComponent->GetSocketTransform(InSocketName) : InOwnerCharacter->GetActorTransform();
}

void UETAttackCollisionComponent::PlayRadialExplosion(AActor* InDamageCauser, AETCharacter* InInstigatorCharacter, const FName InRowName, const FVector& InOrigin, const float InDamage)
{
    const FETRadialAttackCollisionTableRowData* RowData = InDamageCauser ? ETSubsystem::Get<UETGameDataSubsystem>(InDamageCauser).GetRow<FETRadialAttackCollisionTableRowData>(InRowName) : nullptr;
    if (RowData == nullptr || RowData->Radius <= 0.f)
    {
        return;
    }

    // 폭발한 액터가 바로 사라져도 쉐이크가 취소되지 않도록 월드 기준
    TArray<AActor*> OverlapActorArray;
    OverlapRadialAttack(InDamageCauser->GetWorld(), *RowData, InOrigin, InDamageCauser->GetActorRotation(), InDamageCauser, OverlapActorArray);

    AController* InstigatorController = InInstigatorCharacter ? InInstigatorCharacter->GetController() : nullptr;
    for (AActor* OverlapActor : OverlapActorArray)
    {
        const FVector TargetLocation = OverlapActor->GetActorLocation();
        const FVector HitFromDirection = (TargetLocation - InOrigin).GetSafeNormal();
        const FHitResult HitResult(OverlapActor, nullptr, TargetLocation, HitFromDirection);

        if (UGameplayStatics::ApplyPointDamage(OverlapActor, InDamage, HitFromDirection, HitResult, InstigatorController, InDamageCauser, UDamageType::StaticClass()) > 0.f)
        {
            FETHelper::SpawnParticleAtLocation(InDamageCauser, RowData->HitParticle, TargetLocation, FRotator::ZeroRotator, RowData->HitParticleScale);
        }
    }
}

void UETAttackCollisionComponent::PlayRadialAttack(AETCharacter* InOwnerCharacter, const FETRadialAttackCollisionTableRowData& InRowData, const FVector& InOrigin, const FRotator& InCenterParticleRotation)
{
    TArray<AActor*> OverlapActorArray;
    OverlapRadialAttack(this, InRowData, InOrigin, InCenterParticleRotation, InOwnerCharacter, OverlapActorArray);

    for (AActor* OverlapActor : OverlapActorArray)
    {
        const FVector TargetLocation = OverlapActor->GetActorLocation();
        const FHitResult HitResult(OverlapActor, nullptr, TargetLocation, (TargetLocation - InOrigin).GetSafeNormal());

        if (InOwnerCharacter->AttackTarget(OverlapActor, HitResult, InOwnerCharacter->GetAttackDamage()) > 0.f)
        {
            FETHelper::SpawnParticleAtLocation(this, InRowData.HitParticle, TargetLocation, FRotator::ZeroRotator, InRowData.HitParticleScale);
        }
    }
}

void UETAttackCollisionComponent::OverlapRadialAttack(const UObject* InWorldContextObject, const FETRadialAttackCollisionTableRowData& InRowData, const FVector& InOrigin, const FRotator& InCenterParticleRotation, AActor* InIgnoreActor, OUT TArray<AActor*>& OutOverlapActorArray)
{
    TArray<AActor*> IgnoreActorArray;
    if (InIgnoreActor)
    {
        IgnoreActorArray.Add(InIgnoreActor);
    }

    UKismetSystemLibrary::SphereOverlapActors(
        InWorldContextObject,
        InOrigin,
        InRowData.Radius,
        InRowData.ObjectTypeArray,
        AETCharacter::StaticClass(),
        IgnoreActorArray,
        OutOverlapActorArray);

    FETHelper::SpawnParticleAtLocation(InWorldContextObject, InRowData.CenterParticle, InOrigin, InCenterParticleRotation, InRowData.CenterParticleScale);
    if (InRowData.CameraShakeRowName.IsNone() == false)
    {
        // 진원지 기준으로 카메라 쉐이크 테이블 Inner, Outer 반경 거리 감쇠
        ETSubsystem::Get<UETJobSubsystem>(InWorldContextObject).Run(InWorldContextObject, MakeUnique<FETCameraShakeJob>(InRowData.CameraShakeRowName, InOrigin));
    }

#if ENABLE_DRAW_DEBUG
    if (InRowData.DrawDebugType != EDrawDebugTrace::None)
    {
        const bool bPersistent = InRowData.DrawDebugType == EDrawDebugTrace::Persistent;
        const float LifeTime = InRowData.DrawDebugType == EDrawDebugTrace::ForDuration ? InRowData.DrawDebugTime : 0.f;
        const FColor DebugColor = OutOverlapActorArray.IsEmpty() ? FColor::Red : FColor::Green;

        DrawDebugSphere(InWorldContextObject->GetWorld(), InOrigin, InRowData.Radius, 24, DebugColor, bPersistent, LifeTime);
    }
#endif
}

void UETAttackCollisionComponent::PlayMultiRadialAttackWave()
{
    --RemainingMultiRadialAttackWaveCount;

    AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner());
    if (OwnerCharacter && CachedMultiRadialAttackRowData)
    {
        // 웨이브마다 현재 소켓, 오너 위치 기준 (이동하면서 주변 공격 가능)
        const FTransform OriginTransform = GetRadialAttackOriginTransform(OwnerCharacter, MultiRadialAttackData.SocketName);
        const FVector Center = OriginTransform.TransformPosition(MultiRadialAttackData.LocationOffset);
        const float MaxRadius = FMath::Max(MultiRadialAttackData.MinRadius, MultiRadialAttackData.MaxRadius);
        const int32 AttackCount = FMath::RandRange(MultiRadialAttackData.MinAttackCountPerWave, FMath::Max(MultiRadialAttackData.MinAttackCountPerWave, MultiRadialAttackData.MaxAttackCountPerWave));

        for (int32 Index = 0; Index < AttackCount; ++Index)
        {
            // 거리 제곱으로 뽑아야 면적 기준으로 고르게 분포 (그냥 거리로 뽑으면 중심 쪽에 몰림)
            const float Distance = FMath::Sqrt(FMath::FRandRange(FMath::Square(MultiRadialAttackData.MinRadius), FMath::Square(MaxRadius)));
            const float Angle = FMath::FRandRange(0.f, UE_TWO_PI);
            const FVector RandomLocation = Center + FVector(FMath::Cos(Angle) * Distance, FMath::Sin(Angle) * Distance, 0.f);

            PlayRadialAttack(OwnerCharacter, *CachedMultiRadialAttackRowData, FindGroundLocation(RandomLocation), OriginTransform.Rotator());
        }
    }
}

FVector UETAttackCollisionComponent::FindGroundLocation(const FVector& InLocation) const
{
    const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ETMultiRadialAttackGround), false, GetOwner());
    FHitResult HitResult;
    if (GetWorld()->LineTraceSingleByObjectType(HitResult, InLocation + FVector(0.f, 0.f, GroundTraceUpDistance), InLocation - FVector(0.f, 0.f, GroundTraceDownDistance), FCollisionObjectQueryParams(ECC_WorldStatic), QueryParams))
    {
        return HitResult.ImpactPoint;
    }

    return InLocation;
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
                        FETHelper::SpawnParticleAtLocation(this, CachedWeaponCollisionRowData->HitParticle, HitResult.ImpactPoint, FRotator::ZeroRotator, CachedWeaponCollisionRowData->HitParticleScale);
                        if (CachedWeaponCollisionRowData->CameraShakeRowName.IsNone() == false)
                        {
                            ETSubsystem::Get<UETJobSubsystem>(this).Run(this, MakeUnique<FETCameraShakeJob>(CachedWeaponCollisionRowData->CameraShakeRowName));
                        }
                    }
                }
            }
        }
    }
}
