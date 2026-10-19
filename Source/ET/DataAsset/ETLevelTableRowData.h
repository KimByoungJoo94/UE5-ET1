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

	// 레벨명 (체크포인트 이동 목록 등)
	UPROPERTY(EditAnywhere)
	FText DisplayText;

	// 처치 기록이 있으면 레벨 클리어 (보스 행, 비어 있으면 클리어 없음)
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetCharacterRowNameOptions"))
	FName ClearCharacterRowName;

	// 클리어 후 포탈로 이동할 레벨
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetLevelRowNameOptions"))
	FName NextLevelRowName;
};
