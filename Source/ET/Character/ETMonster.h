#pragma once

#include "CoreMinimal.h"
#include "Character/ETCharacter.h"
#include "Interface/ETLockOnInterface.h"
#include "ETMonster.generated.h"

class UWidgetComponent;
class UETDissolveComponent;

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
	virtual AActor* GetAttackTarget() const override;

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
};
