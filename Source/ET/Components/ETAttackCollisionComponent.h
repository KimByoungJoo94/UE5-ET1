#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAsset/ETCollisionTableRowData.h"
#include "ETAttackCollisionComponent.generated.h"

class AETCharacter;

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

	void StartRadialAttackCollision(const FName InRowName);

	// 판정 파티클 출력 (루프 파티클은 일정 시간 뒤 정지, 발사체 등 다른 판정에서도 사용)
	static void SpawnParticle(const UObject* InWorldContextObject, const TSoftObjectPtr<UParticleSystem>& InParticle, const FVector& InLocation, const float InScale);

protected:
	void TraceWeaponCollision();
	void SweepWeaponCollision(AETCharacter* OwnerCharacter, const FVector& StartLocation, const FVector& EndLocation);

private:
	// 데이터 테이블 행 (테이블 에셋이 유지되는 동안 유효)
	const FETWeaponCollisionTableRowData* CachedWeaponCollisionRowData = nullptr;

	TSet<TWeakObjectPtr<AActor>> AlreadHitActorSet;

	bool bWeaponCollisionTracing = false;

	// 이전 틱 소켓 위치 (프레임 사이 무기 궤적 보간용)
	FVector LastStartLocation = FVector::ZeroVector;
	FVector LastEndLocation = FVector::ZeroVector;
	bool bHasLastLocation = false;
};
