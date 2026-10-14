#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystem/ETSubsystem.h"
#include "Engine/DataTable.h"
#include "ETGameDataSubsystem.generated.h"

UCLASS()
class ET_API UETGameDataSubsystem : public UGameInstanceSubsystem
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
	UFUNCTION()
	static TArray<FName> GetGameStatRowNameOptions();

	UFUNCTION()
	static TArray<FName> GetWeaponCollisionRowNameOptions();

	UFUNCTION()
	static TArray<FName> GetRadialAttackCollisionRowNameOptions();

	UFUNCTION()
	static TArray<FName> GetDropItemRowNameOptions();

	UFUNCTION()
	static TArray<FName> GetTimeFreezeRowNameOptions();

	UFUNCTION()
	static TArray<FName> GetCharacterRowNameOptions();

	UFUNCTION()
	static TArray<FName> GetCameraShakeRowNameOptions();

private:
	static TArray<FName> GetRowNameOptions(const UScriptStruct* InRowStruct);

	// /Game/ET/DataTable 폴더(하위 포함)의 모든 DataTable 로드
	static void LoadDataTablesInPath(OUT TArray<UDataTable*>& OutDataTableArray);

private:
	UPROPERTY(Transient)
	TMap<TObjectPtr<UScriptStruct>, TObjectPtr<UDataTable>> DataTableMap;
};
