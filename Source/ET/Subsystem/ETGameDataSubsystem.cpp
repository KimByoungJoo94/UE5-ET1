#include "Subsystem/ETGameDataSubsystem.h"

void UETGameDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DataTableMap.Reset();

	for (const TSoftObjectPtr<UDataTable>& DataTableAsset : DataTableAssetArray)
	{
		UDataTable* DataTable = DataTableAsset.LoadSynchronous();

		checkf(IsValid(DataTable), TEXT("Failed to load DataTable: %s"), *DataTableAsset.ToString());

		UScriptStruct* RowStruct = const_cast<UScriptStruct*>(DataTable->GetRowStruct());

		checkf(IsValid(RowStruct), TEXT("DataTable has invalid RowStruct: %s"), *DataTable->GetName());
		checkf(DataTableMap.Contains(RowStruct) == false, TEXT("Duplicated DataTable RowStruct. RowStruct: %s, DataTable: %s"), *RowStruct->GetName(), *DataTable->GetName());

		DataTableMap.Emplace(RowStruct, DataTable);
	}
}

void UETGameDataSubsystem::Deinitialize()
{
	Super::Deinitialize();
}
