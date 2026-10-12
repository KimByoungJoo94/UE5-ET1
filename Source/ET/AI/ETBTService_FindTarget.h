#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "ETBTService_FindTarget.generated.h"

// 플레이어가 DetectRange 스탯 안에 있으면 대상으로 설정, 벗어나거나 사망하면 해제
UCLASS()
class ET_API UETBTService_FindTarget : public UBTService
{
	GENERATED_BODY()

public:
	UETBTService_FindTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
