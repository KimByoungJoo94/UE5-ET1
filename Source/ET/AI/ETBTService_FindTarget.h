#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTService.h"
#include "ETBTService_FindTarget.generated.h"

// 캐릭터 테이블 AIRowName 의 AI 테이블 탐지 설정으로 플레이어 인식, 해제 (범위는 스폰 볼륨)
// 인식 : Range 는 스폰 볼륨 범위 안 && 탐지 거리 안, Sight 는 여기에 시야 안에서 보일 때만, Volume 은 스폰 볼륨 범위 안이면 무조건
// 해제 : Leash 는 탐지 거리와 스폰 볼륨 범위를 모두 벗어나면 (Volume 은 스폰 볼륨 범위만), Persistent 는 대상 사망 시
// 처음 인식하면 같은 스폰 볼륨 몬스터도 연쇄 인식
UCLASS()
class ET_API UETBTService_FindTarget : public UBTService
{
	GENERATED_BODY()

public:
	UETBTService_FindTarget();

protected:
	virtual void TickNode(UBehaviorTreeComponent& OwnerComp, uint8* NodeMemory, float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, Category = "Blackboard")
	FBlackboardKeySelector TargetActorKey;
};
