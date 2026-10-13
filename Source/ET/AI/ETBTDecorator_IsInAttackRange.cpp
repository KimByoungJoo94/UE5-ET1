#include "AI/ETBTDecorator_IsInAttackRange.h"
#include "Character/ETCharacter.h"
#include "Components/ETGameStatComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Common/ETNames.h"

UETBTDecorator_IsInAttackRange::UETBTDecorator_IsInAttackRange()
{
	NodeName = TEXT("ET Is In Attack Range");
	bNotifyTick = true;

	// 공격 중에는 사거리를 벗어나도 끊지 않고, 추적 중 사거리에 들어오면 공격으로 전환
	FlowAbortMode = EBTFlowAbortMode::LowerPriority;

	TargetActorKey.SelectedKeyName = ETNames::BlackboardKey::TargetActor;
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey), AActor::StaticClass());
}

bool UETBTDecorator_IsInAttackRange::CalculateRawConditionValue(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory) const
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	AETCharacter* OwnerCharacter = AIController ? Cast<AETCharacter>(AIController->GetPawn()) : nullptr;
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	const AActor* TargetActor = BlackboardComponent ? Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (OwnerCharacter == nullptr || TargetActor == nullptr)
	{
		return false;
	}

	const float AttackRange = OwnerCharacter->GetGameStatComponent()->GetGameStat(EETGameStatType::AttackRange).GetCurrentValue();
	return FVector::Dist(OwnerCharacter->GetActorLocation(), TargetActor->GetActorLocation()) <= AttackRange;
}

void UETBTDecorator_IsInAttackRange::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	ConditionalFlowAbort(OwnerComp, EBTDecoratorAbortRequest::ConditionResultChanged);
}
