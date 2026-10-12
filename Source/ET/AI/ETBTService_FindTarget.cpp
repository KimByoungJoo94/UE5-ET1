#include "AI/ETBTService_FindTarget.h"
#include "Character/ETCharacter.h"
#include "Components/ETCharacterStateComponent.h"
#include "Components/ETGameStatComponent.h"
#include "Common/ETGameplayTags.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"

UETBTService_FindTarget::UETBTService_FindTarget()
{
	NodeName = TEXT("ET Find Target");
	Interval = 0.3f;
	RandomDeviation = 0.05f;

	TargetActorKey.SelectedKeyName = TEXT("TargetActor");
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey), AActor::StaticClass());
}

void UETBTService_FindTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const AAIController* AIController = OwnerComp.GetAIOwner();
	AETCharacter* OwnerCharacter = AIController ? Cast<AETCharacter>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (OwnerCharacter == nullptr || BlackboardComponent == nullptr)
	{
		return;
	}

	AETCharacter* PlayerCharacter = Cast<AETCharacter>(UGameplayStatics::GetPlayerPawn(OwnerCharacter, 0));
	const float DetectRange = OwnerCharacter->GetGameStatComponent()->GetGameStat(EETGameStatType::DetectRange).GetCurrentValue();

	const bool bDetected = PlayerCharacter &&
		PlayerCharacter->GetCharacterStateComponent()->IsCurrentState(ETGameplayTags::Character_State_Death) == false &&
		FVector::Dist(OwnerCharacter->GetActorLocation(), PlayerCharacter->GetActorLocation()) <= DetectRange;

	BlackboardComponent->SetValueAsObject(TargetActorKey.SelectedKeyName, bDetected ? PlayerCharacter : nullptr);
}
