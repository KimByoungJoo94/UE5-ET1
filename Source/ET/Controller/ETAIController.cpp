#include "Controller/ETAIController.h"
#include "Character/ETCharacter.h"
#include "DataAsset/ETCharacterTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/ETNames.h"

void AETAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	const AETCharacter* PossessedCharacter = Cast<AETCharacter>(InPawn);
	const FETCharacterTableRowData* CharacterRowData = PossessedCharacter ? ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETCharacterTableRowData>(PossessedCharacter->GetCharacterRowName()) : nullptr;
	UBehaviorTree* BehaviorTree = CharacterRowData ? CharacterRowData->BehaviorTree.LoadSynchronous() : nullptr;
	if (BehaviorTree == nullptr || RunBehaviorTree(BehaviorTree) == false)
	{
		return;
	}

	// 대상을 놓치면 돌아올 위치
	GetBlackboardComponent()->SetValueAsVector(ETNames::BlackboardKey::HomeLocation, InPawn->GetActorLocation());
}
