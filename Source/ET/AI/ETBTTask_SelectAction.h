#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "ETBTTask_SelectAction.generated.h"

// AI 별로 유지되는 노드 메모리
struct FETSelectActionMemory
{
	// 직전에 고른 액션 인덱스 (아직 고른 적 없으면 INDEX_NONE)
	int32 LastActionIndex = INDEX_NONE;
};

// 쿨타임, 비용을 만족하는 액션 중 가중치로 하나를 골라 블랙보드에 인덱스 기록, 후보가 없으면 실패
// 거리는 보지 않음 (ET Move To Action Range 가 액션 사용 거리로 접근, 후퇴), 다른 후보가 있으면 직전에 고른 액션은 제외
UCLASS()
class ET_API UETBTTask_SelectAction : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UETBTTask_SelectAction();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;

protected:
	UPROPERTY(EditAnywhere, Category = "Action", meta = (Categories = "Character.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ActionIndexKey;
};
