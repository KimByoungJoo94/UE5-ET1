// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ETCombatInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UETCombatInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class ET_API IETCombatInterface
{
	GENERATED_BODY()
	
public:
	virtual void OnStartWeaponCollision() {}
	virtual void OnEndWeaponCollision() {}	
	virtual void OnHeavyAttackPause() {}

	virtual void OnHit() = 0;
	virtual void OnDeath() = 0;
};
