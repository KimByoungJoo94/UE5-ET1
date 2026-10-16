#include "AI/ETBTService_FindTarget.h"
#include "Character/ETMonster.h"
#include "Components/ETCharacterStateComponent.h"
#include "Components/ETGameStatComponent.h"
#include "Common/ETGameplayTags.h"
#include "Common/ETNames.h"
#include "DataAsset/ETCharacterTableRowData.h"
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

	const AETSpawnVolume* SpawnVolume = OwnerMonster->GetSpawnVolume();
	const FETCharacterTableRowData* CharacterRowData = ETSubsystem::Get<UETGameDataSubsystem>(OwnerMonster).GetRow<FETCharacterTableRowData>(OwnerMonster->GetCharacterRowName());
	if (ensureMsgf(SpawnVolume, TEXT("Monster must be spawned by AETSpawnVolume. Monster: %s"), *OwnerMonster->GetName()) == false || CharacterRowData == nullptr)
	{
		return;
	}

	AETCharacter* PlayerCharacter = Cast<AETCharacter>(UGameplayStatics::GetPlayerPawn(OwnerMonster, 0));
	if (PlayerCharacter == nullptr || PlayerCharacter->GetCharacterStateComponent()->IsCurrentState(ETGameplayTags::Character_State_Death))
	{
		BlackboardComponent->SetValueAsObject(TargetActorKey.SelectedKeyName, nullptr);
		return;
	}

	const FETCharacterAIData& AIData = CharacterRowData->AIData;
	const float DetectRange = OwnerMonster->GetGameStatComponent()->GetGameStat(EETGameStatType::DetectRange).GetCurrentValue();
	const bool bInDetectRange = FVector::Dist(OwnerMonster->GetActorLocation(), PlayerCharacter->GetActorLocation()) <= DetectRange;
	const bool bInSpawnVolume = SpawnVolume->IsInRange(PlayerCharacter->GetActorLocation());

	bool bTargeting = false;

	if (BlackboardComponent->GetValueAsObject(TargetActorKey.SelectedKeyName) == PlayerCharacter)
	{
		// 인식 중이면 시야는 보지 않음 (가려져도 바로 놓치지 않도록)
		bTargeting = AIData.TargetLoseType == EETTargetLoseType::Persistent || bInDetectRange || bInSpawnVolume;
	}
	else
	{
		bTargeting = bInSpawnVolume && bInDetectRange && (AIData.TargetDetectType == EETTargetDetectType::Range || IsInSight(OwnerMonster, PlayerCharacter, AIData.SightAngle));
	}

	BlackboardComponent->SetValueAsObject(TargetActorKey.SelectedKeyName, bTargeting ? PlayerCharacter : nullptr);
}

bool UETBTService_FindTarget::IsInSight(const AActor* InOwner, const AActor* InTarget, const float InSightAngle)
{
	const FVector ForwardDirection = InOwner->GetActorForwardVector().GetSafeNormal2D();
	const FVector TargetDirection = (InTarget->GetActorLocation() - InOwner->GetActorLocation()).GetSafeNormal2D();
	if (FVector::DotProduct(ForwardDirection, TargetDirection) < FMath::Cos(FMath::DegreesToRadians(InSightAngle * 0.5f)))
	{
		return false;
	}

	// 다른 캐릭터에 가려지는 건 무시하고 지형만 확인
	FVector EyeLocation;
	FRotator EyeRotation;
	InOwner->GetActorEyesViewPoint(EyeLocation, EyeRotation);

	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ETFindTargetSight), false, InOwner);
	return InOwner->GetWorld()->LineTraceTestByObjectType(EyeLocation, InTarget->GetActorLocation(), FCollisionObjectQueryParams(ECC_WorldStatic), QueryParams) == false;
}
