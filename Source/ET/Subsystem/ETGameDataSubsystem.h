#pragma once

#include "CoreMinimal.h"
#include "Subsystem/ETGameSubsystem.h"
#include "Engine/DataTable.h"
#include "ETGameDataSubsystem.generated.h"

UCLASS()
class ET_API UETGameDataSubsystem : public UETGameSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	static UETGameDataSubsystem* Get(const UObject* InWorldContextObject);

	// 월드 컨텍스트로 서브시스템을 찾아 조회 (서브시스템이나 행이 없으면 nullptr)
	template <typename T>
	static const T* GetRow(const UObject* InWorldContextObject, const FName InRowName)
	{
		const UETGameDataSubsystem* GameDataSubsystem = Get(InWorldContextObject);
		return GameDataSubsystem ? GameDataSubsystem->GetRow<T>(InRowName) : nullptr;
	}

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

private:
	static TArray<FName> GetRowNameOptions(const UScriptStruct* InRowStruct);

	// /Game/ET/DataTable 폴더(하위 포함)의 모든 DataTable 로드
	static void LoadDataTablesInPath(OUT TArray<UDataTable*>& OutDataTableArray);

private:
	UPROPERTY(Transient)
	TMap<TObjectPtr<UScriptStruct>, TObjectPtr<UDataTable>> DataTableMap;
};
