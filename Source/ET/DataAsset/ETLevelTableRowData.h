#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETLevelTableRowData.generated.h"

class UWorld;

// 레벨 정보 (다른 테이블에서는 RowName 으로 참조)
USTRUCT()
struct ET_API FETLevelTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> Level;
};
