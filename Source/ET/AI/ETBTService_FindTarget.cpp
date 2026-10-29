#include "AI/ETBTService_FindTarget.h"
#include "Character/ETMonster.h"
#include "Components/ETCharacterStateComponent.h"
#include "Common/ETGameplayTags.h"
#include "Common/ETNames.h"
#include "Common/ETHelper.h"
#include "DataAsset/ETCharacterTableRowData.h"
#include "DataAsset/ETAITableRowData.h"
#include "Spawn/ETSpawnVolume.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "AIController.h"
#include "Kismet/GameplayStatics.h"

UETBTService_FindTarget::UETBTService_FindTarget()
{
	NodeName = TEXT("ET Find Target");
	Interval = 0.3f;
	RandomDeviation = 0.05f;

	TargetActorKey.SelectedKeyName = ETNames::BlackboardKey::TargetActor;
	TargetActorKey.AddObjectFilter(this, GET_MEMBER_NAME_CHECKED(ThisClass, TargetActorKey), AActor::StaticClass());
}

void UETBTService_FindTarget::TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds)
{
	Super::TickNode(OwnerComp, NodeMemory, DeltaSeconds);

	const AAIController* AIController = OwnerComp.GetAIOwner();
	AETMonster* OwnerMonster = AIController ? Cast<AETMonster>(AIController->GetPawn()) : nullptr;
	UBlackboardComponent* BlackboardComponent = OwnerComp.GetBlackboardComponent();
	if (OwnerMonster == nullptr || BlackboardComponent == nullptr)
	{
		return;
	}

	const UETGameDataSubsystem& GameDataSubsystem = ETSubsystem::Get<UETGameDataSubsystem>(OwnerMonster);
	const AETSpawnVolume* SpawnVolume = OwnerMonster->GetHomeSpawnVolume();
	const FETCharacterTableRowData* CharacterRowData = GameDataSubsystem.GetRow<FETCharacterTableRowData>(OwnerMonster->GetCharacterRowName());
	const FETAITableRowData* AIRowData = CharacterRowData ? GameDataSubsystem.GetRow<FETAITableRowData>(CharacterRowData->AIRowName) : nullptr;
	if (ensureMsgf(SpawnVolume, TEXT("Monster must be spawned by AETSpawnVolume. Monster: %s"), *OwnerMonster->GetName()) == false || AIRowData == nullptr)
	{
		return;
	}

	AETCharacter* PlayerCharacter = Cast<AETCharacter>(UGameplayStatics::GetPlayerPawn(OwnerMonster, 0));
	if (PlayerCharacter == nullptr || PlayerCharacter->IsDead())
	{
		OwnerMonster->LoseTarget();
		return;
	}

	// Volume 은 탐지 거리를 쓰지 않음
	const bool bUseDetectRange = AIRowData->TargetDetectType != EETTargetDetectType::Volume;
	const bool bPlayerInDetectRange = bUseDetectRange && FVector::Dist(OwnerMonster->GetActorLocation(), PlayerCharacter->GetActorLocation()) <= AIRowData->DetectRange;
	const bool bPlayerInSpawnVolume = SpawnVolume->IsInRange(PlayerCharacter->GetActorLocation());

	const bool bWasTargeting = BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName) == PlayerCharacter;
	if (bWasTargeting)
	{
		// 인식 중이면 시야는 보지 않음 (가려져도 바로 놓치지 않도록)
		const bool bKeepTargeting = AIRowData->TargetLoseType == EETTargetLoseType::Persistent || bPlayerInDetectRange || bPlayerInSpawnVolume;
		if (bKeepTargeting == false)
		{
			OwnerMonster->LoseTarget();
		}

		return;
	}

	bool bDetected = false;
	switch (AIRowData->TargetDetectType)
	{
	case EETTargetDetectType::Range:
		bDetected = bPlayerInSpawnVolume && bPlayerInDetectRange;
		break;
	case EETTargetDetectType::Sight:
		bDetected = bPlayerInSpawnVolume && bPlayerInDetectRange && FETHelper::IsInSight(OwnerMonster, PlayerCharacter, AIRowData->SightAngle);
		break;
	case EETTargetDetectType::Volume:
		bDetected = bPlayerInSpawnVolume;
		break;
	}

	// 처음 인식하면 등장 연출 후 전투 (인트로가 없거나 이미 재생했으면 무시), 같은 스폰 볼륨 몬스터도 연쇄 인식
	if (bDetected)
	{
		OwnerMonster->DetectTarget(PlayerCharacter, true);
	}
}
