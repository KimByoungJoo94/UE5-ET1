#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ETSpawnVolume.generated.h"

class AETMonster;
class UBillboardComponent;
class USphereComponent;
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

public:
	// 생성한 몬스터를 모두 제거하고 처음 상태로 다시 생성
	void ResetSpawn();

	// 볼륨 중심 기준 범위 구체 안인지
	bool IsInRange(const FVector& InLocation) const;

private:
	UFUNCTION()
	void OnRangeSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 플레이어 캡슐의 진입, 이탈만 처리 (다른 컴포넌트, 몬스터 오버랩 무시)
	bool IsPlayerCapsule(const AActor* InOtherActor, const UPrimitiveComponent* InOtherComp) const;

	// 플레이어 진입 여부를 기록하고 생성한 몬스터의 체력바 표시 갱신
	void SetPlayerInRange(const bool bInRange);

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

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBillboardComponent> SpriteComponent;
#endif

private:
	TArray<TWeakObjectPtr<AETMonster>> SpawnedMonsterArray;

	// 리셋으로 새로 생성한 몬스터에도 현재 상태를 적용하기 위해 기록
	bool bPlayerInRange = false;
};
