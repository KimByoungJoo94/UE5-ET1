#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "ETBTService_FindTarget.generated.h"

// 캐릭터 테이블의 AI 탐지 설정으로 플레이어 인식, 해제 (탐지 거리는 DetectRange 스탯, 범위는 스폰 볼륨)
// 인식 : 스폰 볼륨 범위 안 && 탐지 거리 안 (Sight 는 시야 안에서 보일 때만)
// 해제 : Leash 는 탐지 거리와 스폰 볼륨 범위를 모두 벗어나면, Persistent 는 대상 사망 시
UCLASS()
class ET_API UETBTService_FindTarget : public UBTService
{
	GENERATED_BODY()

public:
	UETBTService_FindTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

private:
	// 정면 기준 시야각 안에 있고 지형에 가려지지 않았는지
	static bool IsInSight(const AActor* InOwner, const AActor* InTarget, const float InSightAngle);

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
