#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAsset/ETCollisionTableRowData.h"
#include "ETAttackCollisionComponent.generated.h"

class AETCharacter;

// 노티파이 스테이트 구간 동안 영역 안 랜덤 위치에 범위 공격을 여러 번 (웨이브마다 현재 소켓, 오너 위치 기준)
USTRUCT()
struct FETMultiRadialAttackData
{
	GENERATED_BODY()

public:
	// 랜덤 위치마다 수행할 범위 공격 하나의 판정
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetRadialAttackCollisionRowNameOptions"))
	FName RadialAttackCollisionRowName;

	// 랜덤 영역 중심 (유효하면 소켓, 비우면 오너 기준 + 로컬 오프셋)
	UPROPERTY(EditAnywhere)
	FName SocketName;

	UPROPERTY(EditAnywhere)
	FVector LocationOffset = FVector::ZeroVector;

	// 영역 중심으로부터 랜덤 거리 (MinRadius 안쪽은 비움)
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float MinRadius = 0.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float MaxRadius = 500.f;

	// 구간 시작부터 균등한 간격으로 수행할 횟수
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	int32 WaveCount = 3;

	// 웨이브마다 동시에 수행할 범위 공격 수
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	int32 MinAttackCountPerWave = 1;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "1"))
	int32 MaxAttackCountPerWave = 3;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETAttackCollisionComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UETAttackCollisionComponent();

protected:
	virtual void BeginPlay() override;

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE bool IsWeaponCollisionTracing() { return bWeaponCollisionTracing; }
	void StartWeaponCollision(const FName InRowName);
	void EndWeaponCollision();

	// 판정 중심은 소켓(없으면 오너) 위치에 로컬 오프셋을 더한 위치 (X 전방, Y 우측, Z 위, 소켓이면 소켓 회전 기준)
	// 중심 파티클 회전은 소켓(없으면 오너) 회전에 회전 오프셋을 더한 값
	void StartRadialAttackCollision(const FName InRowName, const FName InSocketName = NAME_None, const FVector& InLocationOffset = FVector::ZeroVector, const FRotator& InRotationOffset = FRotator::ZeroRotator);

	// 첫 웨이브는 바로 수행, 남은 웨이브는 InDuration 을 WaveCount 로 나눈 간격마다 수행
	void StartMultiRadialAttackCollision(const FETMultiRadialAttackData& InMultiRadialAttackData, const float InDuration);
	void EndMultiRadialAttackCollision();

	// 공격자 캐릭터 없는 범위 폭발 (기믹 등, 같은 편 판정 없이 범위 안 모든 캐릭터에게 데미지)
	// 데미지 Instigator 는 InInstigatorCharacter (맞은 몬스터의 인식, 처치 보상 대상, 없으면 Instigator 없이 데미지)
	static void PlayRadialExplosion(AActor* InDamageCauser, AETCharacter* InInstigatorCharacter, const FName InRowName, const FVector& InOrigin, const float InDamage);

protected:
	void TraceWeaponCollision();
	void SweepWeaponCollision(AETCharacter* OwnerCharacter, const FVector& StartLocation, const FVector& EndLocation);
	// 소켓이 유효하면 소켓, 아니면 오너 Transform
	FTransform GetRadialAttackOriginTransform(const AETCharacter* InOwnerCharacter, const FName InSocketName) const;
	// 중심 위치에서 판정 후 오너의 공격으로 맞은 캐릭터마다 데미지, 피격 파티클
	void PlayRadialAttack(AETCharacter* InOwnerCharacter, const FETRadialAttackCollisionTableRowData& InRowData, const FVector& InOrigin, const FRotator& InCenterParticleRotation);
	// 중심 위치에서 판정, 중심 파티클, 카메라 쉐이크, 디버그 (데미지는 호출한 쪽에서 맞은 캐릭터마다)
	static void OverlapRadialAttack(const UObject* InWorldContextObject, const FETRadialAttackCollisionTableRowData& InRowData, const FVector& InOrigin, const FRotator& InCenterParticleRotation, AActor* InIgnoreActor, OUT TArray<AActor*>& OutOverlapActorArray);

	void PlayMultiRadialAttackWave();
	// 위치 아래 바닥 (바닥이 없으면 그대로)
	FVector FindGroundLocation(const FVector& InLocation) const;

private:
	// 데이터 테이블 행 (테이블 에셋이 유지되는 동안 유효)
	const FETWeaponCollisionTableRowData* CachedWeaponCollisionRowData = nullptr;

	TSet<TWeakObjectPtr<AActor>> AlreadHitActorSet;

	bool bWeaponCollisionTracing = false;

	// 이전 틱 소켓 위치 (프레임 사이 무기 궤적 보간용)
	FVector LastStartLocation = FVector::ZeroVector;
	FVector LastEndLocation = FVector::ZeroVector;
	bool bHasLastLocation = false;

	// 여러 번 범위 공격 진행 상태 (남은 웨이브가 있는 동안 Tick 에서 수행)
	FETMultiRadialAttackData MultiRadialAttackData;
	const FETRadialAttackCollisionTableRowData* CachedMultiRadialAttackRowData = nullptr;
	float MultiRadialAttackInterval = 0.f;
	float MultiRadialAttackElapsedTime = 0.f;
	int32 RemainingMultiRadialAttackWaveCount = 0;
};
