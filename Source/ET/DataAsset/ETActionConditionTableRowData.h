#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DataAsset/ETGameStatTableRowData.h"
#include "ETActionConditionTableRowData.generated.h"

// 액션 몽타주의 사용 조건 (액션 데이터 에셋의 몽타주 항목이 RowName 으로 참조, 값이 비어 있는 조건은 적용하지 않음)
USTRUCT()
struct ET_API FETActionConditionTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 시전 시 소모되는 스탯과 수치 (0 이면 비용 없음)
	UPROPERTY(EditAnywhere, Category = "Common")
	EETGameStatType CostStatType = EETGameStatType::Mana;

	UPROPERTY(EditAnywhere, Category = "Common", meta = (ClampMin = "0.0"))
	float CostValue = 0.f;

	// 재사용 대기 시간 (0 이면 없음, 같은 RowName 을 쓰는 몽타주끼리 공유)
	UPROPERTY(EditAnywhere, Category = "Common", meta = (ClampMin = "0.0"))
	float Cooldown = 0.f;

	// AI 가 액션을 사용할 대상과의 거리 범위 (캡슐 표면 사이 거리, MaxRange 0 이면 거리 제한 없음)
	// 액션을 고른 뒤 범위 안의 랜덤 거리를 향해 접근, 후퇴하고 범위 안에 들어오면 사용
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0.0"))
	float MinRange = 0.f;

	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0.0"))
	float MaxRange = 0.f;

	// AI 가 사용 가능한 액션 중 고를 확률 가중치
	UPROPERTY(EditAnywhere, Category = "AI", meta = (ClampMin = "0.0"))
	float Weight = 1.f;
};
