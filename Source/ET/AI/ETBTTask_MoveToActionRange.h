#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTTaskNode.h"
#include "GameplayTagContainer.h"
#include "ETBTTask_MoveToActionRange.generated.h"

class AAIController;
class AETCharacter;

// AI 별로 유지되는 노드 메모리
struct FETMoveToActionRangeMemory
{
	// 액션 조건의 사용 거리 범위 (MaxRange 0 이면 제한 없음)
	float MinRange = 0.f;
	float MaxRange = 0.f;

	// 범위 안에서 고른 목표 거리 (이 거리를 향해 이동하고 범위 안에 들어오면 성공)
	float DesiredRange = 0.f;

	float ElapsedTime = 0.f;
	float RepathElapsedTime = 0.f;
};

// 블랙보드의 액션 인덱스 액션 조건 거리 범위로 대상을 바라보며 접근, 후퇴 (캡슐 표면 사이 거리 기준)
// 범위 안이면 성공, 경로가 없거나 제한 시간을 넘기면 실패 (실패하면 BT 에서 액션을 다시 고름)
UCLASS()
class ET_API UETBTTask_MoveToActionRange : public UBTTaskNode
{
	GENERATED_BODY()

public:
	UETBTTask_MoveToActionRange();

protected:
	virtual EBTNodeResult::Type ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) override;
	virtual void TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;
	virtual void OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult) override;
	virtual uint16 GetInstanceMemorySize() const override;
	virtual void InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const override;
	virtual void CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const override;

private:
	bool IsInActionRange(const AETCharacter* InOwnerCharacter, const AActor* InTargetActor, const FETMoveToActionRangeMemory* InMemory) const;

	// 대상에서 소유 캐릭터 방향으로 목표 거리만큼 떨어진 위치로 이동 요청 (경로가 없으면 false)
	bool RequestMove(AAIController* InAIController, const AETCharacter* InOwnerCharacter, const AActor* InTargetActor, const FETMoveToActionRangeMemory* InMemory) const;

	// 후퇴할 때 등을 보이지 않도록 이동 중에는 대상을 바라봄
	void SetFaceTarget(AAIController* InAIController, AActor* InTargetActor, const bool bInFaceTarget) const;

protected:
	UPROPERTY(EditAnywhere, Category = "Action", meta = (Categories = "Character.Action"))
	FGameplayTag ActionTag;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;

	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector ActionIndexKey;

	// 이 시간 안에 범위에 들어오지 못하면 실패
	UPROPERTY(EditAnywhere, Category = "Move", meta = (ClampMin = "0.1"))
	float TimeLimit = 3.f;

	// 대상이 움직이므로 목표 위치를 다시 계산하는 간격
	UPROPERTY(EditAnywhere, Category = "Move", meta = (ClampMin = "0.05"))
	float RepathInterval = 0.25f;
};
