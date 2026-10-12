#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ETLockOnInterface.generated.h"

UINTERFACE(MinimalAPI)
class UETLockOnInterface : public UInterface
{
	GENERATED_BODY()
};

// 락온 대상이 될 수 있는 캐릭터 (UETLockOnComponent 탐색 대상)
class ET_API IETLockOnInterface
{
	GENERATED_BODY()

public:
	virtual void OnLockOnTargeted() = 0;
	virtual void OnLockOnTargetReleased() = 0;
};
