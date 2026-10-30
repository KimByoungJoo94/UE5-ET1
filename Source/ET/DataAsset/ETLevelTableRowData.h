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

	// 이 레벨 안에서 처치하면 레벨 클리어 (다른 레벨에서 같은 캐릭터를 처치해도 이 레벨은 클리어되지 않음, 비어 있으면 클리어 없음)
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetCharacterRowNameOptions"))
	FName ClearCharacterRowName;

	// 레벨을 처음 클리어할 때 플레이어에게 지급 (캐릭터 처치 보상과 별개)
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetRewardRowNameOptions"))
	FName ClearRewardRowName;

	// 클리어 후 포탈로 이동할 레벨 (비어 있으면 마지막 레벨이라 엔딩 레벨로 이동)
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetLevelRowNameOptions"))
	FName NextLevelRowName;
};
