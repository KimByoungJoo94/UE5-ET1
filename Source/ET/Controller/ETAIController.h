#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "ETAIController.generated.h"

// 빙의한 캐릭터의 캐릭터 테이블 AIRowName 행에 지정된 BT 실행
UCLASS()
class ET_API AETAIController : public AAIController
{
	GENERATED_BODY()

protected:
	virtual void OnPossess(APawn* InPawn) override;
};
