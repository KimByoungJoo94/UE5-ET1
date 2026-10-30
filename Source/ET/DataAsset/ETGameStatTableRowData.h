#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETGameStatTableRowData.generated.h"

UENUM(BlueprintType)
enum class EETGameStatType : uint8
{
	Health = 0,	
	WalkSpeed,
	SprintSpeed,
	Attack,
	HeavyAttackMaxCharge,
	Mana,
	ManaRegen,
	ManaDrain,
	Momentum,
	MomentumDrain,
	MomentumAttackBonus,
	MomentumGain,	
	StunPoint,
	StunAttack,
	HealFlaskCount,		// 회복 플라스크 사용 가능 횟수 (현재값이 남은 횟수, 체크포인트 휴식 시 충전)
	HealFlaskPercent,	// 회복 플라스크 1회 회복량 (최대 체력의 %, 30 = 30%)
	Max,
};

USTRUCT(BlueprintType)
struct ET_API FETGameStatTableRowData : public FTableRowBase
{
	GENERATED_BODY()
	
public:
	FORCEINLINE const TMap<EETGameStatType, float>& GetBaseGameStatMap() const { return BaseGameStatMap; }
	FORCEINLINE float GetBaseGameStat(const EETGameStatType InGameStatType) const { return BaseGameStatMap.FindRef(InGameStatType); }
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GameStat")
	TMap<EETGameStatType, float> BaseGameStatMap;
};
