#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "ETBTTask_UseAction.generated.h"

// 블랙보드의 액션 인덱스로 액션 재생, 몽타주가 끝나면 성공 (끊기거나 조건 불충족이면 실패)
UCLASS()
class ET_API UETBTTask_UseAction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UETBTTask_UseAction();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void OnActionEnded(bool bInterrupted);

protected:
	UPROPERTY(EditAnywhere, Category = "Action", meta = (Categories = "Character.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, Category = "Action", meta = (Categories = "Character.State"))
	FGameplayTag StateTag;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ActionIndexKey;

private:
	TWeakObjectPtr<UBehaviorTreeComponent> OwnerComponent;
	bool bWaitingActionEnd = false;
};
