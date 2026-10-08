#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ETLockOnComponent.generated.h"

class AETCharacter;

// 휠 버튼 락온 (대상 고정 카메라)
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETLockOnComponent : public UActorComponent
{
	GENERATED_BODY()
	
public:
	UETLockOnComponent();

public:
	// TODO : 락온 구현 전까지 항상 nullptr
	AETCharacter* GetLockOnTarget() const;
};
