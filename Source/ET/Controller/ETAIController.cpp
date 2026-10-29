#include "Controller/ETAIController.h"
#include "Character/ETCharacter.h"
#include "DataAsset/ETCharacterTableRowData.h"
#include "DataAsset/ETAITableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Common/ETNames.h"

void AETAIController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	const UETGameDataSubsystem& GameDataSubsystem = ETSubsystem::Get<UETGameDataSubsystem>(this);
	const AETCharacter* PossessedCharacter = Cast<AETCharacter>(InPawn);
	const FETCharacterTableRowData* CharacterRowData = PossessedCharacter ? GameDataSubsystem.GetRow<FETCharacterTableRowData>(PossessedCharacter->GetCharacterRowName()) : nullptr;
	const FETAITableRowData* AIRowData = CharacterRowData ? GameDataSubsystem.GetRow<FETAITableRowData>(CharacterRowData->AIRowName) : nullptr;
	UBehaviorTree* BehaviorTree = AIRowData ? AIRowData->BehaviorTree.LoadSynchronous() : nullptr;
	if (BehaviorTree == nullptr || RunBehaviorTree(BehaviorTree) == false)
	{
		return;
	}

	// 대상을 놓치면 돌아올 위치
	GetBlackboardComponent()->SetValueAsVector(ETNames::BlackboardKey::HomeLocation, InPawn->GetActorLocation());
}
