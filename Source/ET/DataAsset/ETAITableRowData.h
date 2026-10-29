#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETAITableRowData.generated.h"

class UBehaviorTree;

UENUM()
enum class EETTargetDetectType : uint8
{
	// 스폰 볼륨 안 && 탐지 거리 안
	Range = 0,
	// 스폰 볼륨 안 && 탐지 거리 안 && 시야각 안에서 지형에 가려지지 않음
	Sight,
	// 스폰 볼륨 안이면 거리 상관없이 인식 (보스 등)
	Volume,
};

UENUM()
enum class EETTargetLoseType : uint8
{
	// 탐지 거리와 스폰 볼륨 범위를 모두 벗어나면 해제
	Leash = 0,
	// 대상 사망 시에만 해제
	Persistent,
};

// AI 설정 (캐릭터 테이블 AIRowName 으로 참조, 같은 행을 여러 캐릭터가 공유)
USTRUCT()
struct ET_API FETAITableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere)
	EETTargetDetectType TargetDetectType = EETTargetDetectType::Range;

	UPROPERTY(EditAnywhere)
	EETTargetLoseType TargetLoseType = EETTargetLoseType::Leash;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", EditCondition = "TargetDetectType != EETTargetDetectType::Volume"))
	float DetectRange = 1000.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "360.0", EditCondition = "TargetDetectType == EETTargetDetectType::Sight"))
	float SightAngle = 120.f;
};
