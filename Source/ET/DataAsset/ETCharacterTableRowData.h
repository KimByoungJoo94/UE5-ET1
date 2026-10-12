#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETCharacterTableRowData.generated.h"

class UETCharacterActionDataAsset;

UENUM()
enum class EETCharacterType : uint8
{
	Player = 0,
	Monster,
	Max,
};

// 캐릭터별 데이터 (캐릭터는 RowName 만 가지고 조회)
USTRUCT()
struct ET_API FETCharacterTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EETCharacterType CharacterType = EETCharacterType::Monster;

	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetGameStatRowNameOptions"))
	FName BaseGameStatRowName;

	// 테이블은 게임 시작 시 모두 로드되므로 몽타주까지 같이 올라오지 않도록 Soft 참조
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UETCharacterActionDataAsset> ActionDataAsset;
};
