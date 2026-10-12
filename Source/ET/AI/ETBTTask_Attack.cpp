#include "AI/ETBTTask_Attack.h"
#include "Character/ETCharacter.h"
#include "Common/ETGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UETBTTask_Attack::UETBTTask_Attack()
{
	NodeName = TEXT("ET Attack");

	// 몽타주 종료 콜백에서 BT 컴포넌트를 찾아야 하므로 AI 별 인스턴스 생성
	bCreateNodeInstance = true;

	TargetActorKey.SelectedKeyName = TEXT("TargetActor");
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey), AActor::StaticClass());

	ActionTag = ETGameplayTags::Character_Action_ComboAttack;
	StateTag = ETGameplayTags::Character_State_Attacking;
}

EBTNodeResult::Type UETBTTask_Attack::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	AETCharacter* OwnerCharacter = AIController ? Cast<AETCharacter>(AIController->GetPawn()) : nullptr;
	if (OwnerCharacter == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	// TODO : 회전 연출이 필요하면 보간 회전으로 교체
	if (const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent())
	{
		if (const AActor* TargetActor = Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName)))
		{
			const FVector ToTarget = (TargetActor->GetActorLocation() - OwnerCharacter->GetActorLocation()).GetSafeNormal2D();
			if (ToTarget.IsNearlyZero() == false)
			{
				OwnerCharacter->SetActorRotation(ToTarget.Rotation());
			}
		}
	}

	OwnerComponent = &OwnerComp;
	bWaitingActionEnd = true;

	if (OwnerCharacter->PlayAction(ActionTag, ActionIndex, StateTag, FOnActionEnded::CreateUObject(this, &ThisClass::OnActionEnded)) == false)
	{
		bWaitingActionEnd = false;
		return EBTNodeResult::Failed;
	}

	return EBTNodeResult::InProgress;
}

EBTNodeResult::Type UETBTTask_Attack::AbortTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
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

void UETBTTask_Attack::OnActionEnded(bool bInterrupted)
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
