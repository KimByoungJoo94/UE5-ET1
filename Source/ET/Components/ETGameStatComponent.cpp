#include "Components/ETGameStatComponent.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

const FETGameStat FETGameStat::Invalid(EETGameStatType::Max);

UETGameStatComponent::UETGameStatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = true;
	bWantsInitializeComponent = true;
}

void UETGameStatComponent::InitializeComponent()
{
	Super::InitializeComponent();

	GenerateGameStat();
}

void UETGameStatComponent::BeginPlay()
{
	Super::BeginPlay();
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

	UpdateOverTime(DeltaTime);
	BroadcastDirtyGameStat();
}

const FETGameStat& UETGameStatComponent::GetGameStat(const EETGameStatType InGameStatType) const
{
	if (const FETGameStat* FoundGameStat = GameStatMap.Find(InGameStatType))
	{
		return *FoundGameStat;
	}
	
	return FETGameStat::Invalid;
}

void UETGameStatComponent::AddDepletedValue(const EETGameStatType InGameStatType, const float InValue)
{
	if (FETGameStat* FoundGameStat = GameStatMap.Find(InGameStatType))
	{
		const float MaxValue = FoundGameStat->GetMaxValue();
		const float OldCurrentValue = FoundGameStat->GetCurrentValue();
		const float NewCurrentValue = FMath::Clamp(OldCurrentValue + InValue, 0.f, MaxValue);

		FoundGameStat->SetDepletedValue(NewCurrentValue - MaxValue);

		if (FMath::IsNearlyEqual(OldCurrentValue, NewCurrentValue) == false)
		{
			DirtyGameStatFlags |= (1ull << static_cast<uint8>(InGameStatType));
		}
	}
}

bool UETGameStatComponent::HasEnoughCurrentValue(const EETGameStatType InGameStatType, const float InValue) const
{
	const FETGameStat& GameStat = GetGameStat(InGameStatType);

	return GameStat.IsValid() && GameStat.GetCurrentValue() >= InValue;
}

void UETGameStatComponent::StartIncreaseOverTime(const EETGameStatType InGameStatType, const EETGameStatType InRateStatType)
{
	StartOverTime(InGameStatType, InRateStatType, true);
}

void UETGameStatComponent::StartDecreaseOverTime(const EETGameStatType InGameStatType, const EETGameStatType InRateStatType)
{
	StartOverTime(InGameStatType, InRateStatType, false);
}

void UETGameStatComponent::StopOverTime(const EETGameStatType InGameStatType)
{
	OverTimeDataMap.Remove(InGameStatType);
}

void UETGameStatComponent::StartOverTime(const EETGameStatType InGameStatType, const EETGameStatType InRateStatType, const bool bInIncrease)
{
	if (GameStatMap.Contains(InGameStatType) == false || GameStatMap.Contains(InRateStatType) == false)
	{
		return;
	}

	if (const FETGameStatOverTimeData* FoundOverTimeData = OverTimeDataMap.Find(InGameStatType))
	{
		if (FoundOverTimeData->RateStatType == InRateStatType && FoundOverTimeData->bIncrease == bInIncrease)
		{
			return;
		}
	}

	FETGameStatOverTimeData OverTimeData;
	OverTimeData.RateStatType = InRateStatType;
	OverTimeData.bIncrease = bInIncrease;
	OverTimeDataMap.Add(InGameStatType, OverTimeData);
}

void UETGameStatComponent::UpdateOverTime(const float InDeltaTime)
{	
	for (const TPair<EETGameStatType, FETGameStatOverTimeData>& OverTimePair : OverTimeDataMap)
	{
		const FETGameStat& GameStat = GetGameStat(OverTimePair.Key);
		const float Rate = FMath::Max(GetGameStat(OverTimePair.Value.RateStatType).GetCurrentValue(), 0.f);

		if (GameStat.IsValid() == false || FMath::IsNearlyZero(Rate))
		{
			continue;
		}

		if (OverTimePair.Value.bIncrease)
		{
			if (GameStat.GetCurrentValue() < GameStat.GetMaxValue())
			{
				AddDepletedValue(OverTimePair.Key, Rate * InDeltaTime);
			}
		}
		else
		{
			if (GameStat.GetCurrentValue() > 0.f)
			{
				AddDepletedValue(OverTimePair.Key, -Rate * InDeltaTime);
			}
		}
	}
}

FOnUpdateGameStat& UETGameStatComponent::GetOnUpdateGameStatDelegate(const EETGameStatType InGameStatType)
{
	check(InGameStatType < EETGameStatType::Max);

	return OnUpdateGameStatDelegateArray[static_cast<int32>(InGameStatType)];
}

void UETGameStatComponent::BroadcastDirtyGameStat()
{
	if (DirtyGameStatFlags == 0)
	{
		return;
	}

	// Broadcast 중 새로 Dirty 된 스탯은 다음 Tick 에서 처리
	const uint64 BroadcastFlags = DirtyGameStatFlags;
	DirtyGameStatFlags = 0;

	for (int32 Index = 0; Index < static_cast<int32>(EETGameStatType::Max); ++Index)
	{
		if ((BroadcastFlags & (1ull << Index)) == 0)
		{
			continue;
		}

		const EETGameStatType GameStatType = static_cast<EETGameStatType>(Index);
		OnUpdateGameStatDelegateArray[Index].Broadcast(GetGameStat(GameStatType));
	}
}
