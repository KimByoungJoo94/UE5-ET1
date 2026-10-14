#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "ETDefeatInterface.generated.h"

UINTERFACE(MinimalAPI)
class UETDefeatInterface : public UInterface
{
	GENERATED_BODY()
};

// 쓰러짐 연출이 끝난 시점에 반응하는 캐릭터 (UETAnimNotify_Defeat 에서 호출, 반응은 캐릭터마다 다름)
class ET_API IETDefeatInterface
{
	GENERATED_BODY()

public:
	virtual void OnDefeated() = 0;
};
