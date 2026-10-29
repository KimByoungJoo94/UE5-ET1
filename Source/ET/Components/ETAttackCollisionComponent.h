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

	void StartRadialAttackCollision(const FName InRowName, const FName InSocketName = NAME_None);

protected:
	void TraceWeaponCollision();
	void SweepWeaponCollision(AETCharacter* OwnerCharacter, const FVector& StartLocation, const FVector& EndLocation);
	// 진원지가 있으면 카메라 쉐이크 테이블 Inner, Outer 반경으로 거리 감쇠
	void PlayCameraShake(const FName InCameraShakeRowName, const FVector* InEpicenter = nullptr);

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
