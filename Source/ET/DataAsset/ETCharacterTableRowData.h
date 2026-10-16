#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETCharacterTableRowData.generated.h"

class UETCharacterActionDataAsset;
class UBehaviorTree;
class AETCharacter;

UENUM()
enum class EETCharacterType : uint8
{
	Player = 0,
	Monster,
	Max,
};

UENUM()
enum class EETTargetDetectType : uint8
{	
	Range = 0,
	Sight,
};

UENUM()
enum class EETTargetLoseType : uint8
{	
	Leash = 0,
	Persistent,
};

// AI 설정 (BT, 대상 탐지, 탐지 거리는 DetectRange 스탯)
USTRUCT()
struct ET_API FETCharacterAIData
{
	GENERATED_BODY()

public:
	// AI 가 실행할 BT (플레이어는 비워 둠, 잡몹은 공용 BT 지정)
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere)
	EETTargetDetectType TargetDetectType = EETTargetDetectType::Range;

	UPROPERTY(EditAnywhere)
	EETTargetLoseType TargetLoseType = EETTargetLoseType::Leash;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "360.0", EditCondition = "TargetDetectType == EETTargetDetectType::Sight"))
	float SightAngle = 120.f;
};

// 캐릭터별 데이터 (캐릭터는 RowName 만 가지고 조회)
USTRUCT()
struct ET_API FETCharacterTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, Category = "Common")
	EETCharacterType CharacterType = EETCharacterType::Monster;

	// 스폰 볼륨이 생성할 캐릭터 BP (생성 시 이 행의 RowName 을 지정)
	UPROPERTY(EditAnywhere, Category = "Common")
	TSoftClassPtr<AETCharacter> CharacterClass;

	UPROPERTY(EditAnywhere, Category = "Common", meta = (GetOptions = "ET.ETGameDataSubsystem.GetGameStatRowNameOptions"))
	FName BaseGameStatRowName;

	// 테이블은 게임 시작 시 모두 로드되므로 몽타주까지 같이 올라오지 않도록 Soft 참조
	UPROPERTY(EditAnywhere, Category = "Common")
	TSoftObjectPtr<UETCharacterActionDataAsset> ActionDataAsset;

	UPROPERTY(EditAnywhere, Category = "AI")
	FETCharacterAIData AIData;
};
