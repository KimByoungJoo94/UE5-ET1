#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETCheckpointTableRowData.generated.h"

// 체크포인트 정보 (위치는 레벨에 배치한 AETCheckpoint 가 가짐)
USTRUCT()
struct ET_API FETCheckpointTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetLevelRowNameOptions"))
	FName LevelRowName;

	// 지역명
	UPROPERTY(EditAnywhere)
	FText DisplayText;
};
