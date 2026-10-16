#pragma once

#include "CoreMinimal.h"
#include "Character/ETCharacter.h"
#include "Interface/ETLockOnInterface.h"
#include "ETMonster.generated.h"

class UWidgetComponent;
class UETDissolveComponent;
class AETSpawnVolume;

UCLASS()
class ET_API AETMonster : public AETCharacter, public IETLockOnInterface
{
	GENERATED_BODY()

public:
	AETMonster();

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Die() override;

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual AActor* GetAttackTarget() const override;

	AETSpawnVolume* GetSpawnVolume() const;
	// 생성한 스폰 볼륨이 FinishSpawning 전에 지정
	void SetSpawnVolume(AETSpawnVolume* InSpawnVolume);

	// 스폰 볼륨이 플레이어 진입, 이탈 시 호출 (사망 후에는 항상 숨김)
	void SetHealthBarVisible(const bool bVisible);

	// IETLockOnInterface
	virtual void OnLockOnTargeted() override;
	virtual void OnLockOnTargetReleased() override;
	// ~IETLockOnInterface

protected:
	// 디졸브 완료 후 사라짐 처리 (풀링 / 리스폰 도입 시 여기만 교체)
	void Disappear();

	void OnDissolveFinished();

protected:
	UPROPERTY(VisibleAnywhere, Category = "ET")
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET")
	TObjectPtr<UWidgetComponent> LockOnWidgetComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET|Component")
	TObjectPtr<UETDissolveComponent> DissolveComponent;

private:
	TWeakObjectPtr<AETSpawnVolume> SpawnVolume;
};
