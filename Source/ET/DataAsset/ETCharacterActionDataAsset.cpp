#include "DataAsset/ETCharacterActionDataAsset.h"

const FETCharacterActionData* UETCharacterActionDataAsset::GetActionData(const FGameplayTag& InGameplayTag) const
{	
	return GameplayTagActionDataMap.Find(InGameplayTag);
}

UAnimMontage* UETCharacterActionDataAsset::GetAnimMontage(const FGameplayTag& InGameplayTag, const int32 InIndex) const
{
	if (const FETCharacterActionData* FoundActionData = GameplayTagActionDataMap.Find(InGameplayTag))
	{
		if (FoundActionData->MontageArray.IsValidIndex(InIndex))
		{
			return FoundActionData->MontageArray[InIndex];
		}
	}

	return nullptr;
}

const TArray<TObjectPtr<UAnimMontage>>& UETCharacterActionDataAsset::GetAnimMontageArray(const FGameplayTag& InGameplayTag) const
{
	if (const FETCharacterActionData* FoundActionData = GameplayTagActionDataMap.Find(InGameplayTag))
	{
		return FoundActionData->MontageArray;
	}

	return EmptyArray;
}


