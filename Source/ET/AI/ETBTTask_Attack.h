#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "ETBTTask_Attack.generated.h"

// 대상 방향으로 돌아서 액션 재생, 몽타주가 끝나면 성공 (끊기면 실패)
UCLASS()
class ET_API UETBTTask_Attack : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UETBTTask_Attack();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual EBTNodeResult::Type AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;

	void OnActionEnded(bool bInterrupted);

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Action", meta = (Categories = "Character.Action"))
	FGameplayTag ActionTag;

	// INDEX_NONE(-1) 이면 액션 몽타주 중 랜덤
	UPROPERTY(EditAnywhere, Category = "Action")
	int32 ActionIndex = INDEX_NONE;

	UPROPERTY(EditAnywhere, Category = "Action", meta = (Categories = "Character.State"))
	FGameplayTag StateTag;

private:	
	TWeakObjectPtr<UBehaviorTreeComponent> OwnerComponent;
	bool bWaitingActionEnd = false;
};
