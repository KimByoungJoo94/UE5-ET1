#include "Components/ETLockOnComponent.h"
#include "Character/ETCharacter.h"

UETLockOnComponent::UETLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

AETCharacter* UETLockOnComponent::GetLockOnTarget() const
{
	return nullptr;
}
