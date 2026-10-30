#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ETSpawnVolume.generated.h"

class AETMonster;
class AETPlayer;
class UBillboardComponent;
class USphereComponent;
class UStaticMeshComponent;
class UMaterialInterface;
struct FETSpawnVolumeCharacterData;

// 스폰 볼륨 테이블 행의 캐릭터를 볼륨 기준 상대 위치에 생성 (몬스터는 반드시 스폰 볼륨으로 배치)
UCLASS()
class ET_API AETSpawnVolume : public AActor
{
	GENERATED_BODY()

public:
	AETSpawnVolume();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

public:
	// 생성한 몬스터를 모두 제거하고 처음 상태로 다시 생성 (테이블 bRespawnOnReset 이 꺼져 있으면 무시)
	void ResetSpawn();

	// 런타임에 생성할 때 BeginPlay 전에 지정 (SpawnActorDeferred 후 FinishSpawning 전)
	FORCEINLINE void SetSpawnVolumeRowName(const FName InSpawnVolumeRowName) { SpawnVolumeRowName = InSpawnVolumeRowName; }

	// 볼륨 중심 기준 범위 구체 안인지
	bool IsInRange(const FVector& InLocation) const;

	// 생성한 몬스터가 Die 에서 호출 (모두 처치하면 잠금 해제)
	void OnSpawnedMonsterDied();

	// 생성한 몬스터가 대상을 인식하면 호출 (같은 볼륨의 나머지 몬스터도 연쇄 인식)
	void AlertSpawnedMonsters(AActor* InTarget);

	// 순간이동 기믹 목적지 후보 (살아 있는 몬스터가 있고 보스가 없으며 플레이어가 범위 밖)
	bool IsTeleportDestination(const AETPlayer* InPlayer) const;

	// 잠금 시작 거리의 원 위에서 내비메시 위이고 캡슐이 겹치지 않는 위치 (볼륨 중심을 바라봄, 못 찾으면 false)
	bool FindTeleportTransform(const AETPlayer* InPlayer, OUT FTransform& OutTransform) const;

private:
	UFUNCTION()
	void OnRangeSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 플레이어 캡슐의 진입, 이탈만 처리 (다른 컴포넌트, 몬스터 오버랩 무시)
	bool IsPlayerCapsule(const AActor* InOtherActor, const UPrimitiveComponent* InOtherComp) const;

	bool HasAliveSpawnedMonster() const;

	// 잠금 대상 플레이어가 있고 살아 있는 몬스터가 있을 때만 Tick (잠금 전 진입 대기, 잠금 후 위치 보정)
	void UpdateLockTickEnabled();

	// 잠금 경계 반지름 (캡슐이 경계 안쪽에 머물도록 캡슐 반지름만큼 줄임)
	float GetLockDistance(const AETPlayer* InPlayer) const;

	void BeginLock(AETPlayer* InPlayer);
	void EndLock();

	// 경계 밖으로 나간 만큼 경계 위로 되돌리고 바깥 방향 속도 제거
	void KeepPlayerInLockRange(AETPlayer* InPlayer) const;

	// 잠금 중 카메라까지 범위 안에 들어오면 경계 표시 (한 번 표시하면 잠금 해제 때까지 유지)
	void TryShowLockBoundary(const AETPlayer* InPlayer);

	void SpawnCharacters();
	void DestroySpawnedCharacters();

	// 테이블 상대 위치, 회전을 볼륨 기준으로 바꾸고 바닥에 맞춘 스폰 트랜스폼 (회전은 Yaw 만)
	FTransform GetSpawnTransform(const FETSpawnVolumeCharacterData& InSpawnCharacterData, const float InCapsuleHalfHeight) const;

	// 위치 아래 바닥에 캡슐 바닥이 닿는 위치 (바닥이 없으면 그대로)
	FVector FindGroundLocation(const FVector& InLocation, const float InCapsuleHalfHeight) const;

#if WITH_EDITOR
	// 에디터 월드에서 범위와 스폰 위치 표시 (테이블 수정 후에는 볼륨을 움직이거나 속성을 바꾸면 갱신)
	void UpdateEditorPreview();
#endif

protected:
	UPROPERTY(EditAnywhere, Category = "ET", meta = (GetOptions = "ET.ETGameDataSubsystem.GetSpawnVolumeRowNameOptions"))
	FName SpawnVolumeRowName;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> RootSceneComponent;

	// 반지름은 테이블 Range (Pawn 오버랩)
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> RangeSphereComponent;

	// 잠금 중 범위 경계 표시 (반지름은 테이블 Range)
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> LockBoundaryMeshComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBillboardComponent> SpriteComponent;
#endif

private:
	TArray<TWeakObjectPtr<AETMonster>> SpawnedMonsterArray;

	// 범위 밖에서 잠금이 풀리면 잠금 대상 해제하기 위해 기록
	bool bPlayerInRange = false;

	// 테이블 리셋 옵션 (생성 시 기록)
	bool bRespawnOnReset = true;

	// 테이블 잠금 옵션 (생성 시 기록)
	bool bLockPlayerInRange = false;
	float LockEnterMargin = 0.f;

	UPROPERTY()
	TObjectPtr<UMaterialInterface> LockBoundaryMaterial;

	// 잠금 옵션이 켜진 볼륨에 들어온 플레이어 (잠금 중에는 범위 구체를 벗어나도 유지)
	TWeakObjectPtr<AETPlayer> LockTargetPlayer;

	bool bPlayerLocked = false;
};
