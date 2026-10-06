#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Common/ETGameplayTags.h"
#include "ETCharacterActionDataAsset.generated.h"

class UAnimMontage;

USTRUCT(BlueprintType)
struct FETCharacterActionData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<TObjectPtr<UAnimMontage>> MontageArray;
};

UCLASS()
class ET_API UETCharacterActionDataAsset : public UPrimaryDataAsset
{
	GENERATED_BODY()
	
public:
	const FETCharacterActionData* GetActionData(const FGameplayTag& InGameplayTag) const;
	UAnimMontage* GetAnimMontage(const FGameplayTag& InGameplayTag, const int32 InIndex) const;
	const TArray<TObjectPtr<UAnimMontage>>& GetAnimMontageArray(const FGameplayTag& InGameplayTag) const;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TMap<FGameplayTag, FETCharacterActionData> GameplayTagActionDataMap;

private:
	const TArray<TObjectPtr<UAnimMontage>> EmptyArray;
};
