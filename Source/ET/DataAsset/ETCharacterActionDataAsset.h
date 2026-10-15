#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Common/ETGameplayTags.h"
#include "ETCharacterActionDataAsset.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FETCharacterActionMontageData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UAnimMontage> Montage;

	// 비용, 쿨타임 등 사용 조건 (비어 있으면 조건 없음)
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetActionConditionRowNameOptions"))
	FName ActionConditionRowName;
};

USTRUCT(BlueprintType)
struct FETCharacterActionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TArray<FETCharacterActionMontageData> MontageDataArray;

	// 비어 있지 않으면 현재 상태가 이 중 하나일 때만 행동 가능 (Idle 에서만 가능 같은 행동용)
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer AllowedStateContainer;

	// 현재 상태가 이 중 하나면 행동 불가
	UPROPERTY(EditAnywhere)
	FGameplayTagContainer BlockedStateContainer;
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
