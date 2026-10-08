#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ETGameSubsystem.h"
#include "Engine/DataTable.h"
#include "DataAsset/ETGameStatTableRowData.h"
#include "DataAsset/ETCollisionTableRowData.h"
#include "DataAsset/ETDropItemTableRowData.h"
#include "ETGameDataSubsystem.generated.h"

UCLASS(Config = Game)
class ET_API UETGameDataSubsystem : public UETGameSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	template <typename T>
	const T* GetRow(const FName InRowName) const
	{
		const UDataTable* DataTable = GetDataTable<T>();

		if (DataTable == nullptr)
		{
			return nullptr;
		}

		return DataTable->FindRow<T>(InRowName, TEXT("UETGameDataSubsystem::GetRow"));
	}

	template <typename T>
	const UDataTable* GetDataTable() const
	{
		static_assert(TIsDerivedFrom<T, FTableRowBase>::IsDerived, "T must derive from FTableRowBase.");

		UScriptStruct* RowStruct = T::StaticStruct();

		const TObjectPtr<UDataTable>* FoundDataTable = DataTableMap.Find(RowStruct);
		if (FoundDataTable == nullptr)
		{
			return nullptr;
		}

		return FoundDataTable->Get();
	}

public:
	// 에디터 드롭다운(GetOptions) 용 RowName 목록
	UFUNCTION() static TArray<FName> GetGameStatRowNameOptions() { return GetRowNameOptions<FETGameStatTableRowData>(); }
	UFUNCTION() static TArray<FName> GetWeaponCollisionRowNameOptions() { return GetRowNameOptions<FETWeaponCollisionTableRowData>(); }
	UFUNCTION() static TArray<FName> GetRadialAttackCollisionRowNameOptions() { return GetRowNameOptions<FETRadialAttackCollisionTableRowData>(); }
	UFUNCTION() static TArray<FName> GetDropItemRowNameOptions() { return GetRowNameOptions<FETDropItemTableRowData>(); }

private:
	template <typename T>
	static TArray<FName> GetRowNameOptions()
	{
		static_assert(TIsDerivedFrom<T, FTableRowBase>::IsDerived, "T must derive from FTableRowBase.");

		// 에디터에는 서브시스템 인스턴스가 없으므로 Config 에 등록된 테이블을 CDO 에서 직접 로드
		for (const TSoftObjectPtr<UDataTable>& DataTableAsset : GetDefault<UETGameDataSubsystem>()->DataTableAssetArray)
		{
			const UDataTable* DataTable = DataTableAsset.LoadSynchronous();
			if (DataTable && DataTable->GetRowStruct() == T::StaticStruct())
			{
				return DataTable->GetRowNames();
			}
		}

		return TArray<FName>();
	}

private:
	UPROPERTY(Config)
	TArray<TSoftObjectPtr<UDataTable>> DataTableAssetArray;

	UPROPERTY(Transient)
	TMap<TObjectPtr<UScriptStruct>, TObjectPtr<UDataTable>> DataTableMap;
};
