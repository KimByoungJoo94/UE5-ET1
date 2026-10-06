#include "Components/ETGameStatComponent.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"

const FETGameStat FETGameStat::Invalid(EETGameStatType::Max);

UETGameStatComponent::UETGameStatComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
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
		const float NewCurrentValue = FMath::Clamp(FoundGameStat->GetCurrentValue() + InValue, 0.f, MaxValue);

		FoundGameStat->SetDepletedValue(NewCurrentValue - MaxValue);
		OnUpdateGameStat.Broadcast(*FoundGameStat);
	}
}
