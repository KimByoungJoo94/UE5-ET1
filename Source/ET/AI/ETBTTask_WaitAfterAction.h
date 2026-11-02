#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "ETBTTask_WaitAfterAction.generated.h"

// Use Action 뒤에서 사용. 선택된 액션 조건의 WaitTime ± RandomDeviation 만큼 대기
// 조건 행이 없거나 대기시간이 0 이면 즉시 성공
UCLASS()
class ET_API UETBTTask_WaitAfterAction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UETBTTask_WaitAfterAction();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const override;

	UPROPERTY(EditAnywhere, Category = "Action", meta = (Categories = "Character.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ActionIndexKey;
};
