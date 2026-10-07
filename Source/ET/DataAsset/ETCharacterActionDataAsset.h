#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Common/ETGameplayTags.h"
#include "DataAsset/ETGameStatTableRowData.h"
#include "ETCharacterActionDataAsset.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FETCharacterActionMontageData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Montage;

	// 시전 시 소모되는 스탯과 수치 (0 이면 비용 없음)
	UPROPERTY(EditAnywhere)
	EETGameStatType CostStatType = EETGameStatType::Mana;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float CostValue = 0.f;
};

USTRUCT(BlueprintType)
struct FETCharacterActionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FETCharacterActionMontageData> MontageDataArray;
};

UCLASS()
class ET_API UETCharacterActionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()

public:
	const FETCharacterActionData* GetActionData(const FGameplayTag& InGameplayTag) const;
	const FETCharacterActionMontageData* GetActionMontageData(const FGameplayTag& InGameplayTag, const int32 InIndex) const;
	const TArray<FETCharacterActionMontageData>& GetActionMontageDataArray(const FGameplayTag& InGameplayTag) const;
	UAnimMontage* GetAnimMontage(const FGameplayTag& InGameplayTag, const int32 InIndex) const;

protected:
	UPROPERTY(EditAnywhere)
	TMap<FGameplayTag, FETCharacterActionData> GameplayTagActionDataMap;

private:
	const TArray<FETCharacterActionMontageData> EmptyArray;
};
