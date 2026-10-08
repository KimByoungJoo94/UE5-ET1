#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "DataAsset/ETGameStatTableRowData.h"
#include "ETDropItemTableRowData.generated.h"

// 채워진 항목을 모두 획득자에게 적용
USTRUCT()
struct ET_API FETDropItemTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 영구 능력치 상승 (스탯 종류 -> 증가량)
	UPROPERTY(EditAnywhere)
	TMap<EETGameStatType, float> PermanentGameStatMap;

	// TODO : 아이템 시스템 추가 후 ItemRewardArray
};
