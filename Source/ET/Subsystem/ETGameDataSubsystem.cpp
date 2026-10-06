#include "Subsystem/ETGameDataSubsystem.h"
#include "Engine/DataTable.h"
#include "DataAsset/ETGameStatTableRowBase.h"

void UETGameDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	BaseGameStatDataTable = LoadObject<UDataTable>(nullptr, TEXT("/Script/Engine.DataTable'/Game/ET/DataTable/DT_BaseGameStat.DT_BaseGameStat'"));
}

void UETGameDataSubsystem::Deinitialize()
{
	BaseGameStatDataTable = nullptr;

	Super::Deinitialize();
}

const FETGameStatTableRowBase* UETGameDataSubsystem::GetGameStatRow(const FName InRowName) const
{
	if (BaseGameStatDataTable == nullptr)
	{
		return nullptr;
	}

	return BaseGameStatDataTable->FindRow<FETGameStatTableRowBase>(InRowName, TEXT("GetGameStatRow"));
}