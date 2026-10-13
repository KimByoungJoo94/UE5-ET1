#pragma once

#include "CoreMinimal.h"
#include "Character/ETCharacter.h"
#include "Interface/ETLockOnInterface.h"
#include "ETMonster.generated.h"

class UWidgetComponent;

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
	UPROPERTY(VisibleAnywhere, Category = "ET")
	TObjectPtr<UWidgetComponent> HealthBarWidgetComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET")
	TObjectPtr<UWidgetComponent> LockOnWidgetComponent;
};
