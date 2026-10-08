#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ETSkillTableRowData.generated.h"

// 시전자 주변 반경 안에서 시야(원뿔) 안의 대상을 가까운 순으로 정지
USTRUCT()
struct ET_API FETTimeFreezeTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	FETTimeFreezeTableRowData();

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float SearchRadius = 1500.f;

	// 카메라 정면 기준 (카메라가 없으면 시전자 정면)
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float ViewHalfAngle = 45.f;

	// 0 이면 범위 안 전체
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0"))
	int32 MaxTargetCount = 1;

	// 0 이하이면 Unfreeze 호출 전까지 정지 유지
	UPROPERTY(EditAnywhere)
	float FreezeDuration = 3.f;

	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeArray;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::None;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float DrawDebugTime = 2.f;
};
