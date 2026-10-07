#include "DataAsset/ETCharacterActionDataAsset.h"

const FETCharacterActionData* UETCharacterActionDataAsset::GetActionData(const FGameplayTag& InGameplayTag) const
{
	return GameplayTagActionDataMap.Find(InGameplayTag);
}

const FETCharacterActionMontageData* UETCharacterActionDataAsset::GetActionMontageData(const FGameplayTag& InGameplayTag, const int32 InIndex) const
{
	if (const FETCharacterActionData* FoundActionData = GameplayTagActionDataMap.Find(InGameplayTag))
	{
		if (FoundActionData->MontageDataArray.IsValidIndex(InIndex))
		{
			return &FoundActionData->MontageDataArray[InIndex];
		}
	}

	return nullptr;
}

const TArray<FETCharacterActionMontageData>& UETCharacterActionDataAsset::GetActionMontageDataArray(const FGameplayTag& InGameplayTag) const
{
	if (const FETCharacterActionData* FoundActionData = GameplayTagActionDataMap.Find(InGameplayTag))
	{
		return FoundActionData->MontageDataArray;
	}

	return EmptyArray;
}

UAnimMontage* UETCharacterActionDataAsset::GetAnimMontage(const FGameplayTag& InGameplayTag, const int32 InIndex) const
{
	if (const FETCharacterActionMontageData* FoundMontageData = GetActionMontageData(InGameplayTag, InIndex))
	{
		return FoundMontageData->Montage;
	}

	return nullptr;
}
