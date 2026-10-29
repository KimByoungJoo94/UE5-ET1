#include "AI/ETBTTask_SelectAction.h"
#include "Character/ETCharacter.h"
#include "Common/ETGameplayTags.h"
#include "Common/ETNames.h"
#include "DataAsset/ETActionConditionTableRowData.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"

UETBTTask_SelectAction::UETBTTask_SelectAction()
{
	NodeName = TEXT("ET Select Action");

	ActionTag = ETGameplayTags::Character_Action_ActiveSkill;

	TargetActorKey.SelectedKeyName = ETNames::BlackboardKey::TargetActor;
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey), AActor::StaticClass());

	ActionIndexKey.SelectedKeyName = ETNames::BlackboardKey::ActionIndex;
	ActionIndexKey.AddIntFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, ActionIndexKey));
}

EBTNodeResult::Type UETBTTask_SelectAction::ExecuteTask(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory)
{
	const AAIController* AIController = OwnerComp.GetAIOwner();
	const AETCharacter* OwnerCharacter = AIController ? Cast<AETCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	const AActor* TargetActor = BlackboardComponent ? Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName)) : nullptr;
	if (OwnerCharacter == nullptr || TargetActor == nullptr || OwnerCharacter->CanPlayAction(ActionTag) == false)
	{
		return EBTNodeResult::Failed;
	}

	// 쿨타임, 비용을 만족하는 액션만 후보 (거리는 이동 태스크에서 맞춤)
	TArray<TPair<int32, float>> CandidateArray;
	float TotalWeight = 0.f;

	const int32 MontageNum = OwnerCharacter->GetActionMontageNum(ActionTag);
	for (int32 Index = 0; Index < MontageNum; ++Index)
	{
		const FETCharacterActionMontageData* MontageData = OwnerCharacter->FindPlayableActionMontageData(ActionTag, Index);
		if (MontageData == nullptr)
		{
			continue;
		}

		const FETActionConditionTableRowData* ConditionRowData = OwnerCharacter->GetActionConditionRowData(MontageData);
		const float Weight = ConditionRowData ? ConditionRowData->Weight : 1.f;
		if (Weight <= 0.f)
		{
			continue;
		}

		CandidateArray.Emplace(Index, Weight);
		TotalWeight += Weight;
	}

	if (CandidateArray.IsEmpty())
	{
		return EBTNodeResult::Failed;
	}

	// 같은 액션이 연속으로 나오지 않도록 다른 후보가 있으면 직전 액션 제외
	FETSelectActionMemory* Memory = CastInstanceNodeMemory<FETSelectActionMemory>(NodeMemory);
	if (CandidateArray.Num() > 1)
	{
		const int32 LastCandidateIndex = CandidateArray.IndexOfByPredicate([Memory](const TPair<int32, float>& InCandidate) { return InCandidate.Key == Memory->LastActionIndex; });
		if (LastCandidateIndex != INDEX_NONE)
		{
			TotalWeight -= CandidateArray[LastCandidateIndex].Value;
			CandidateArray.RemoveAt(LastCandidateIndex);
		}
	}

	int32 SelectedIndex = CandidateArray.Last().Key;
	float RandomWeight = FMath::FRandRange(0.f, TotalWeight);
	for (const TPair<int32, float>& Candidate : CandidateArray)
	{
		RandomWeight -= Candidate.Value;
		if (RandomWeight <= 0.f)
		{
			SelectedIndex = Candidate.Key;
			break;
		}
	}

	BlackboardComponent->SetValueAsInt(ActionIndexKey.SelectedKeyName, SelectedIndex);

	Memory->LastActionIndex = SelectedIndex;

	return EBTNodeResult::Succeeded;
}

uint16 UETBTTask_SelectAction::GetInstanceMemorySize() const
{
	return sizeof(FETSelectActionMemory);
}

void UETBTTask_SelectAction::InitializeMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryInit::Type InitType) const
{
	InitializeNodeMemory<FETSelectActionMemory>(NodeMemory, InitType);
}

void UETBTTask_SelectAction::CleanupMemory(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, EBTMemoryClear::Type CleanupType) const
{
	CleanupNodeMemory<FETSelectActionMemory>(NodeMemory, CleanupType);
}
