#include "Components/ETGameStatComponent.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

const FETGameStat FETGameStat::Invalid(EETGameStatType::Max);

UETGameStatComponent::UETGameStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UETGameStatComponent::BeginPlay()
{
	Super::BeginPlay();

	GenerateGameStat();
}

void UETGameStatComponent::GenerateGameStat()
{
	if (UETGameDataSubsystem* GameDataSubsystem = UGameInstance::GetSubsystem<UETGameDataSubsystem>(UGameplayStatics::GetGameInstance(this)))
	{
		if (const FETGameStatTableRowData* GameStatRow = GameDataSubsystem->GetRow<FETGameStatTableRowData>(BaseGameStatRowName))
		{
			for (int32 Index = 0; Index < static_cast<int32>(EETGameStatType::Max); ++Index)
			{
				const EETGameStatType GameStatType = static_cast<EETGameStatType>(Index);

				FETGameStat GameStat(GameStatType, GameStatRow->GetBaseGameStat(GameStatType));
				GameStatMap.Add(GameStatType, GameStat);
			}
		}
	}
}

void UETGameStatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

const FETGameStat& UETGameStatComponent::GetGameStat(const EETGameStatType InGameStatType) const
{
	if (const FETGameStat* FoundGameStat = GameStatMap.Find(InGameStatType))
	{
		return *FoundGameStat;
	}
	
	return FETGameStat::Invalid;
}

