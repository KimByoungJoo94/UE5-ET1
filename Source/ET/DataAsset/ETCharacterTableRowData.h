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
	// 처치 기록을 세이브에 남김 (최초 처치 보상, 레벨 클리어 판정)
	Boss,
	Max,
};

namespace ETCharacterType
{
	// 플레이어와 몬스터(보스 포함)만 서로 적 (같은 편은 공격이 닿아도 데미지 없음)
	inline bool IsHostile(const EETCharacterType InAttackerType, const EETCharacterType InTargetType)
	{
		return (InAttackerType == EETCharacterType::Player) != (InTargetType == EETCharacterType::Player);
	}
}

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
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY(EditAnywhere)
	EETTargetDetectType TargetDetectType = EETTargetDetectType::Range;

	UPROPERTY(EditAnywhere)
	EETTargetLoseType TargetLoseType = EETTargetLoseType::Leash;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0", ClampMax = "360.0", EditCondition = "TargetDetectType == EETTargetDetectType::Sight"))
	float SightAngle = 120.f;
};

// 처치(사망) 시 처리
USTRUCT()
struct ET_API FETCharacterDefeatData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetNoticeWidgetRowNameOptions"))
	FName NoticeWidgetRowName;
		
	// 처치할 때마다 처치한 캐릭터에게 지급
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetRewardRowNameOptions"))
	FName RepeatRewardRowName;

	// 최초 처치 시 한 번만 지급 (값이 있으면 처치 기록 대상)
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetRewardRowNameOptions"))
	FName OnceRewardRowName;

	// 처치 순간 전체 슬로 (실제 시간 기준, 0 이하면 슬로 없음)
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float SlowDuration = 0.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float SlowTimeDilation = 0.2f;
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

	UPROPERTY(EditAnywhere, Category = "Defeat")
	FETCharacterDefeatData DefeatData;
};
