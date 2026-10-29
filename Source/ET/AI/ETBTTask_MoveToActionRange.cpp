#include "AI/ETBTTask_MoveToActionRange.h"
#include "Character/ETCharacter.h"
#include "Common/ETGameplayTags.h"
#include "Common/ETNames.h"
#include "Common/ETHelper.h"
#include "DataAsset/ETActionConditionTableRowData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Navigation/PathFollowingComponent.h"
#include "AIController.h"

UETBTTask_MoveToActionRange::UETBTTask_MoveToActionRange()
{
	NodeName = TEXT("ET Move To Action Range");
	bNotifyTick = true;
	bNotifyTaskFinished = true;

	ActionTag = ETGameplayTags::Character_Action_ActiveSkill;

	TargetActorKey.SelectedKeyName = ETNames::BlackboardKey::TargetActor;
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey), AActor::StaticClass());

	ActionIndexKey.SelectedKeyName = ETNames::BlackboardKey::ActionIndex;
	ActionIndexKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, ActionIndexKey));
}

EBTNodeResult::Type UETBTTask_MoveToActionRange::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	const AETCharacter* OwnerCharacter = AIController ? Cast<AETCharacter>(AIController->GetPawn()) : nullptr;
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	AActor* TargetActor = BlackboardComponent ? Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (OwnerCharacter == nullptr || TargetActor == nullptr)
	{
		return EBTNodeResult::Failed;
	}

	const int32 ActionIndex = BlackboardComponent->GetValueAsInt(ActionIndexKey.SelectedKeyName);
	const FETActionConditionTableRowData* ConditionRowData = OwnerCharacter->GetActionConditionRowData(OwnerCharacter->FindPlayableActionMontageData(ActionTag, ActionIndex));

	FETMoveToActionRangeMemory* Memory = CastInstanceNodeMemory<FETMoveToActionRangeMemory>(NodeMemory);
	Memory->MinRange = ConditionRowData ? ConditionRowData->MinRange : 0.f;
	Memory->MaxRange = ConditionRowData && ConditionRowData->MaxRange > 0.f ? ConditionRowData->MaxRange : UE_BIG_NUMBER;
	// 거리 제한이 없으면 MinRange 까지만 접근
	Memory->DesiredRange = Memory->MaxRange < UE_BIG_NUMBER ? FMath::FRandRange(Memory->MinRange, Memory->MaxRange) : Memory->MinRange;
	Memory->ElapsedTime = 0.f;
	Memory->RepathElapsedTime = 0.f;

	if (IsInActionRange(OwnerCharacter, TargetActor, Memory))
	{
		return EBTNodeResult::Succeeded;
	}

	SetFaceTarget(AIController, TargetActor, true);

	return RequestMove(AIController, OwnerCharacter, TargetActor, Memory) ? EBTNodeResult::InProgress : EBTNodeResult::Failed;
}

void UETBTTask_MoveToActionRange::TickTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	AAIController* AIController = OwnerComp.GetAIOwner();
	const AETCharacter* OwnerCharacter = AIController ? Cast<AETCharacter>(AIController->GetPawn()) : nullptr;
	const UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	const AActor* TargetActor = BlackboardComponent ? Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (OwnerCharacter == nullptr || TargetActor == nullptr)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		return;
	}

	FETMoveToActionRangeMemory* Memory = CastInstanceNodeMemory<FETMoveToActionRangeMemory>(NodeMemory);
	Memory->ElapsedTime += DeltaSeconds;
	Memory->RepathElapsedTime += DeltaSeconds;

	if (IsInActionRange(OwnerCharacter, TargetActor, Memory))
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Succeeded);
	}
	else if (Memory->ElapsedTime >= TimeLimit)
	{
		FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
	}
	else if (Memory->RepathElapsedTime >= RepathInterval)
	{
		Memory->RepathElapsedTime = 0.f;

		if (RequestMove(AIController, OwnerCharacter, TargetActor, Memory) == false)
		{
			FinishLatentTask(OwnerComp, EBTNodeResult::Failed);
		}
	}
}

void UETBTTask_MoveToActionRange::OnTaskFinished(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTNodeResult::Type TaskResult)
{
	// 성공, 실패, 중단 모두 이동을 멈추고 기본 회전(이동 방향)으로 복구
	if (AAIController* AIController = OwnerComp.GetAIOwner())
	{
		AIController->StopMovement();
		SetFaceTarget(AIController, nullptr, false);
	}

	Super::OnTaskFinished(OwnerComp, NodeMemory, TaskResult);
}

bool UETBTTask_MoveToActionRange::IsInActionRange(const AETCharacter* InOwnerCharacter, const AActor* InTargetActor, const FETMoveToActionRangeMemory* InMemory) const
{
	const float Distance = FETHelper::GetCollisionDistance2D(InOwnerCharacter, InTargetActor);
	return Distance >= InMemory->MinRange && Distance <= InMemory->MaxRange;
}

bool UETBTTask_MoveToActionRange::RequestMove(AAIController* InAIController, const AETCharacter* InOwnerCharacter, const AActor* InTargetActor, const FETMoveToActionRangeMemory* InMemory) const
{
	float OwnerRadius = 0.f;
	float OwnerHalfHeight = 0.f;
	float TargetRadius = 0.f;
	float TargetHalfHeight = 0.f;
	InOwnerCharacter->GetSimpleCollisionCylinder(OwnerRadius, OwnerHalfHeight);
	InTargetActor->GetSimpleCollisionCylinder(TargetRadius, TargetHalfHeight);

	// 겹쳐 있어서 방향이 없으면 뒤로 물러남
	FVector DirectionFromTarget = (InOwnerCharacter->GetActorLocation() - InTargetActor->GetActorLocation()).GetSafeNormal2D();
	if (DirectionFromTarget.IsNearlyZero())
	{
		DirectionFromTarget = -InOwnerCharacter->GetActorForwardVector().GetSafeNormal2D();
	}

	// 목표 거리는 캡슐 표면 사이 기준이므로 두 반지름을 더해 중심 위치로 변환
	const FVector GoalLocation = InTargetActor->GetActorLocation() + DirectionFromTarget * (InMemory->DesiredRange + OwnerRadius + TargetRadius);

	return InAIController->MoveToLocation(GoalLocation, -1.f, false, true, true, true) != EPathFollowingRequestResult::Failed;
}

void UETBTTask_MoveToActionRange::SetFaceTarget(AAIController* InAIController, AActor* InTargetActor, const bool bInFaceTarget) const
{
	if (bInFaceTarget)
	{
		InAIController->SetFocus(InTargetActor);
	}
	else
	{
		InAIController->ClearFocus(EAIFocusPriority::Gameplay);
	}

	// 몬스터 기본은 이동 방향으로 회전, 바라보는 동안은 컨트롤러(포커스) 방향으로 회전
	const ACharacter* Character = Cast<ACharacter>(InAIController->GetPawn());
	if (UCharacterMovementComponent* CharacterMovementComponent = Character ? Character->GetCharacterMovement() : nullptr)
	{
		CharacterMovementComponent->bOrientRotationToMovement = bInFaceTarget == false;
		CharacterMovementComponent->bUseControllerDesiredRotation = bInFaceTarget;
	}
}

uint16 UETBTTask_MoveToActionRange::GetInstanceMemorySize() const
{
	return sizeof(FETMoveToActionRangeMemory);
}

void UETBTTask_MoveToActionRange::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	InitializeNodeMemory<FETMoveToActionRangeMemory>(NodeMemory, InitType);
}

void UETBTTask_MoveToActionRange::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
	CleanupNodeMemory<FETMoveToActionRangeMemory>(NodeMemory, CleanupType);
}
