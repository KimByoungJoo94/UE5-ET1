#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETGameStatTableRowData.generated.h"

UENUM(BlueprintType)
enum class EETGameStatType : uint8
{
	Health = 0,	
	Speed,
	// Level,
	// Stamina,
	// MaxStamina,
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
