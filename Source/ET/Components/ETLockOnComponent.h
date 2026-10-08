#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ETLockOnComponent.generated.h"

class AETCharacter;

// 휠 버튼 락온 (대상 고정 카메라)
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UETLockOnComponent();

public:
	// TODO : 락온 구현 전까지 항상 nullptr
	AETCharacter* GetLockOnTarget() const;

	// 오너 주변 반경 안에서 카메라 시야(원뿔) 안의 캐릭터를 가까운 순으로 탐색 (InMaxCount 0 이면 전체)
	void FindTargetsInView(const float InRadius, const float InViewHalfAngle, const int32 InMaxCount, OUT TArray<AETCharacter*>& OutTargetArray) const;

protected:
	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeArray;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::None;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float DrawDebugTime = 2.f;
};
