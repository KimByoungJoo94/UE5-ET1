#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAsset/ETCollisionTableRowData.h"
#include "ETAttackCollisionComponent.generated.h"

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

protected:
	void TraceWeaponCollision();

private:
	// 데이터 테이블 행 (테이블 에셋이 유지되는 동안 유효)
	const FETWeaponCollisionTableRowData* WeaponCollisionRowData = nullptr;

	TSet<TWeakObjectPtr<AActor>> AlreadHitActorSet;

	bool bWeaponCollisionTracing = false;
};
