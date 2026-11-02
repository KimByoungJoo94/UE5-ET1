#include "AI/ETBTTask_WaitAfterAction.h"
#include "Character/ETCharacter.h"
#include "Common/ETGameplayTags.h"
#include "Common/ETNames.h"
#include "DataAsset/ETActionConditionTableRowData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UETBTTask_WaitAfterAction::UETBTTask_WaitAfterAction()
{
	NodeName = TEXT("ET Wait After Action");
	bNotifyTick = true;
	bTickIntervals = true;

	ActionTag = ETGameplayTags::Character_Action_ActiveSkill;
	ActionIndexKey.SelectedKeyName = ETNames::BlackboardKey::ActionIndex;
	ActionIndexKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, ActionIndexKey));
}

EBTNodeResult::Type UETBTTask_WaitAfterAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const AETCharacter* OwnerCharacter = AIController ? Cast<AETCharacter>(AIController->GetPawn()) : nullptr;
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (OwnerCharacter == nullptr || BlackboardComponent == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	const int32 ActionIndex = BlackboardComponent->GetValueAsInt(ActionIndexKey.SelectedKeyName);
	// 사용 직후 쿨타임이나 비용이 부족해도 방금 사용한 액션의 설정은 읽어야 함
	const FETCharacterActionMontageData* MontageData = OwnerCharacter->GetActionMontageData(ActionTag, ActionIndex);
	if (MontageData == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	const FETActionConditionTableRowData* ConditionRowData = OwnerCharacter->GetActionConditionRowData(MontageData);
	if (ConditionRowData == nullptr)
	{
		return EBTNodeResult::Succeeded;
	}

	const float WaitTime = FMath::Max(0.f, ConditionRowData->WaitTime);
	const float RandomDeviation = FMath::Max(0.f, ConditionRowData->RandomDeviation);
	const float RemainingWaitTime = FMath::FRandRange(FMath::Max(0.f, WaitTime - RandomDeviation), WaitTime + RandomDeviation);
	if (RemainingWaitTime <= 0.f)
	{
		return EBTNodeResult::Succeeded;
	}

	// UE Wait 와 같이 AI 별 특수 노드 메모리에 다음 틱 시각을 저장, 매 프레임 틱하지 않음
	SetNextTickTime(NodeMemory, RemainingWaitTime);
	return EBTNodeResult::InProgress;
}

void UETBTTask_WaitAfterAction::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
}

void UETBTTask_WaitAfterAction::DescribeRuntimeValues(const UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTDescriptionVerbosity::Type Verbosity, TArray<FString>& Values) const
{
	Super::DescribeRuntimeValues(OwnerComp, NodeMemory, Verbosity, Values);
	const FBTTaskMemory* TaskMemory = GetSpecialNodeMemory<FBTTaskMemory>(NodeMemory);
	if (TaskMemory->NextTickRemainingTime > 0.f)
	{
		Values.Add(FString::Printf(TEXT("remaining: %ss"), *FString::SanitizeFloat(TaskMemory->NextTickRemainingTime)));
	}
}
