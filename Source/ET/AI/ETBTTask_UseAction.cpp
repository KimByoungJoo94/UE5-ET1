#include "AI/ETBTTask_UseAction.h"
#include "Character/ETCharacter.h"
#include "Common/ETGameplayTags.h"
#include "Common/ETNames.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UETBTTask_UseAction::UETBTTask_UseAction()
{
	NodeName = TEXT("ET Use Action");

	// 몽타주 종료 콜백에서 BT 컴포넌트를 찾아야 하므로 AI 별 인스턴스 생성
	bCreateNodeInstance = true;

	ActionTag = ETGameplayTags::Character_Action_ActiveSkill;
	StateTag = ETGameplayTags::Character_State_UsingSkill;

	ActionIndexKey.SelectedKeyName = ETNames::BlackboardKey::ActionIndex;
	ActionIndexKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, ActionIndexKey));
}

EBTNodeResult::Type UETBTTask_UseAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	AETCharacter* OwnerCharacter = AIController ? Cast<AETCharacter>(AIController->GetPawn()) : nullptr;
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (OwnerCharacter == nullptr || BlackboardComponent == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// 타겟 방향 회전은 몽타주의 ETAnimNotifyState_RotateToTarget 구간에서 처리
	OwnerComponent = &OwnerComp;
	bWaitingActionEnd = true;

	const int32 ActionIndex = BlackboardComponent->GetValueAsInt(ActionIndexKey.SelectedKeyName);
	if (OwnerCharacter->PlayAction(ActionTag, ActionIndex, StateTag, FOnActionEnded::CreateUObject(this, &ThisClass::OnActionEnded)) == false)
	{
		bWaitingActionEnd = false;
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UETBTTask_UseAction::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	// 몽타주 정지 시 바로 호출되는 종료 콜백은 무시
	bWaitingActionEnd = false;

	const AAIController* AIController = OwnerComp.GetAIOwner();
	if (AETCharacter* OwnerCharacter = AIController ? Cast<AETCharacter>(AIController->GetPawn()) : nullptr)
	{
		OwnerCharacter->StopAnimMontage();
	}

	return EBTNodeResult::Aborted;
}

void UETBTTask_UseAction::OnActionEnded(bool bInterrupted)
{
	if (bWaitingActionEnd == false)
	{
		return;
	}

	bWaitingActionEnd = false;

	if (UBehaviorTreeComponent* BehaviorTreeComponent = OwnerComponent.Get())
	{
		FinishLatentTask(*BehaviorTreeComponent, bInterrupted ? EBTNodeResult::Failed : EBTNodeResult::Succeeded);
	}
}
