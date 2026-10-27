#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ETProjectile.generated.h"

class AETCharacter;
class UParticleSystemComponent;
class UProjectileMovementComponent;
class USphereComponent;
struct FETProjectileTableRowData;

// 발사체 테이블 행 기준으로 직선, 포물선 이동 (캐릭터에 맞으면 발사자 공격, 지형에 맞으면 소멸 / 관통 없음)
UCLASS()
class ET_API AETProjectile : public AActor
{
	GENERATED_BODY()

public:
	AETProjectile();

protected:
	virtual void PreInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// 생성 시 FinishSpawning 전에만 지정 (PreInitializeComponents 에서 행 데이터 적용, 발사자는 Instigator)
	FORCEINLINE void SetProjectileRowName(const FName InProjectileRowName) { ProjectileRowName = InProjectileRowName; }

private:
	UFUNCTION()
	void OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnProjectileStop(const FHitResult& ImpactResult);

	// 대상이 있으면 공격, 지형이거나 실제 데미지가 들어가면 히트 파티클 출력 후 소멸
	void ProcessHit(const FHitResult& InHitResult, AETCharacter* InTargetCharacter);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USphereComponent> SphereComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UProjectileMovementComponent> ProjectileMovementComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UParticleSystemComponent> RootParticleSystemComponent;

private:
	FName ProjectileRowName;

	// 데이터 테이블 행 (테이블 에셋이 유지되는 동안 유효)
	const FETProjectileTableRowData* CachedProjectileRowData = nullptr;

	// 발사자가 먼저 사라져도 데미지는 들어가도록 발사 시점 값 기록
	TWeakObjectPtr<AETCharacter> OwnerCharacter;
	float AttackDamage = 0.f;

	// 한 틱에 여러 대상과 겹쳐도 한 번만 처리
	bool bHit = false;
};
