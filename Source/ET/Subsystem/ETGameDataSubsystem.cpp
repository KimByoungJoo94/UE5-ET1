#include "Subsystem/ETGameDataSubsystem.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "DataAsset/ETGameStatTableRowData.h"
#include "DataAsset/ETCollisionTableRowData.h"
#include "DataAsset/ETDropItemTableRowData.h"
#include "DataAsset/ETSkillTableRowData.h"
#include "DataAsset/ETCharacterTableRowData.h"

void UETGameDataSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	DataTableMap.Reset();

	TArray<UDataTable*> DataTableArray;
	LoadDataTablesInPath(DataTableArray);

	for (UDataTable* DataTable : DataTableArray)
	{
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

UETGameDataSubsystem* UETGameDataSubsystem::Get(const UObject* InWorldContextObject)
{
	return UGameInstance::GetSubsystem<UETGameDataSubsystem>(UGameplayStatics::GetGameInstance(InWorldContextObject));
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

TArray<FName> UETGameDataSubsystem::GetCharacterRowNameOptions()
{
	return GetRowNameOptions(FETCharacterTableRowData::StaticStruct());
}

TArray<FName> UETGameDataSubsystem::GetRowNameOptions(const UScriptStruct* InRowStruct)
{
	// 에디터에는 서브시스템 인스턴스가 없으므로 폴더의 테이블을 직접 로드
	TArray<UDataTable*> DataTableArray;
	LoadDataTablesInPath(DataTableArray);

	for (const UDataTable* DataTable : DataTableArray)
	{
		if (DataTable->GetRowStruct() == InRowStruct)
		{
			return DataTable->GetRowNames();
		}
	}

	return TArray<FName>();
}

void UETGameDataSubsystem::LoadDataTablesInPath(OUT TArray<UDataTable*>& OutDataTableArray)
{
	// 이 폴더에 있는 DataTable 은 별도 등록 없이 RowStruct 별로 자동 등록
	const FName DataTablePath(TEXT("/Game/ET/DataTable"));

	OutDataTableArray.Reset();

	IAssetRegistry& AssetRegistry = FAssetRegistryModule::GetRegistry();

#if WITH_EDITOR
	// 에디터 시작 직후에는 백그라운드 스캔이 끝나지 않았을 수 있으므로 해당 폴더만 동기 스캔
	AssetRegistry.ScanPathsSynchronous({ DataTablePath.ToString() });
#endif

	FARFilter Filter;
	Filter.PackagePaths.Add(DataTablePath);
	Filter.bRecursivePaths = true;
	Filter.ClassPaths.Add(UDataTable::StaticClass()->GetClassPathName());
	Filter.bRecursiveClasses = true;

	TArray<FAssetData> AssetDataArray;
	AssetRegistry.GetAssets(Filter, AssetDataArray);

	for (const FAssetData& AssetData : AssetDataArray)
	{
		UDataTable* DataTable = Cast<UDataTable>(AssetData.GetAsset());

		checkf(IsValid(DataTable), TEXT("Failed to load DataTable: %s"), *AssetData.GetObjectPathString());

		OutDataTableArray.Add(DataTable);
	}
}
