#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "ETBTTask_SelectAction.generated.h"

// 지금 사용 가능한 액션 중 가중치로 하나를 골라 블랙보드에 인덱스와 사용 거리(MaxRange) 기록, 후보가 없으면 실패
UCLASS()
class ET_API UETBTTask_SelectAction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UETBTTask_SelectAction();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Action", meta = (Categories = "Character.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ActionIndexKey;

	// MoveTo 의 AcceptableRadius 에 연결 (MaxRange 0 이면 이동 없이 바로 사용)
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ActionRangeKey;
};
