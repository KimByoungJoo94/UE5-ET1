#include "Subsystem/ETGameDataSubsystem.h"
#include "DataAsset/ETGameStatTableRowData.h"
#include "DataAsset/ETCollisionTableRowData.h"
#include "DataAsset/ETDropItemTableRowData.h"
#include "DataAsset/ETSkillTableRowData.h"

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

TArray<FName> UETGameDataSubsystem::GetGameStatRowNameOptions()
{
	return GetRowNameOptions(FETGameStatTableRowData::StaticStruct());
}

TArray<FName> UETGameDataSubsystem::GetWeaponCollisionRowNameOptions()
{
	return GetRowNameOptions(FETWeaponCollisionTableRowData::StaticStruct());
}

TArray<FName> UETGameDataSubsystem::GetRadialAttackCollisionRowNameOptions()
{
	return GetRowNameOptions(FETRadialAttackCollisionTableRowData::StaticStruct());
}

TArray<FName> UETGameDataSubsystem::GetDropItemRowNameOptions()
{
	return GetRowNameOptions(FETDropItemTableRowData::StaticStruct());
}

TArray<FName> UETGameDataSubsystem::GetTimeFreezeRowNameOptions()
{
	return GetRowNameOptions(FETTimeFreezeTableRowData::StaticStruct());
}

TArray<FName> UETGameDataSubsystem::GetRowNameOptions(const UScriptStruct* InRowStruct)
{
	// 에디터에는 서브시스템 인스턴스가 없으므로 Config 에 등록된 테이블을 CDO 에서 직접 로드
	for (const TSoftObjectPtr<UDataTable>& DataTableAsset : GetDefault<UETGameDataSubsystem>()->DataTableAssetArray)
	{
		const UDataTable* DataTable = DataTableAsset.LoadSynchronous();
		if (DataTable && DataTable->GetRowStruct() == InRowStruct)
		{
			return DataTable->GetRowNames();
		}
	}

	return TArray<FName>();
}
