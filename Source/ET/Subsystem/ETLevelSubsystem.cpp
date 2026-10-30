#include "Subsystem/ETLevelSubsystem.h"
#include "Character/ETCharacter.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Subsystem/ETSaveSubsystem.h"
#include "DataAsset/ETLevelTableRowData.h"
#include "DataAsset/ETCheckpointTableRowData.h"
#include "Common/ETCommonSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

DEFINE_LOG_CATEGORY_STATIC(LogLevelTravel, Log, All);

void UETLevelSubsystem::RestartLevel()
{
	const FString CurrentLevelName = UGameplayStatics::GetCurrentLevelName(this);
	UE_LOG(LogLevelTravel, Log, TEXT("RestartLevel: %s"), *CurrentLevelName);

	UGameplayStatics::OpenLevel(this, FName(CurrentLevelName));
}

void UETLevelSubsystem::OpenLevel(const FName InLevelRowName)
{
	const FETLevelTableRowData* LevelRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETLevelTableRowData>(InLevelRowName);
	if (ensureMsgf(LevelRowData && LevelRowData->Level.IsNull() == false, TEXT("Invalid LevelRowName: %s"), *InLevelRowName.ToString()) == false)
	{
		return;
	}

	UE_LOG(LogLevelTravel, Log, TEXT("OpenLevel: %s -> %s"), *InLevelRowName.ToString(), *LevelRowData->Level.ToString());

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelRowData->Level);
}

void UETLevelSubsystem::OpenTitleLevel()
{
	const TSoftObjectPtr<UWorld>& TitleLevel = GetDefault<UETCommonSettings>()->Level.TitleLevel;
	if (ensureMsgf(TitleLevel.IsNull() == false, TEXT("TitleLevel is not set in ET Common Settings.")) == false)
	{
		return;
	}

	UE_LOG(LogLevelTravel, Log, TEXT("OpenTitleLevel: %s"), *TitleLevel.ToString());

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, TitleLevel);
}

void UETLevelSubsystem::OpenEndingLevel()
{
	const TSoftObjectPtr<UWorld>& EndingLevel = GetDefault<UETCommonSettings>()->Level.EndingLevel;
	if (ensureMsgf(EndingLevel.IsNull() == false, TEXT("EndingLevel is not set in ET Common Settings.")) == false)
	{
		return;
	}

	UE_LOG(LogLevelTravel, Log, TEXT("OpenEndingLevel: %s"), *EndingLevel.ToString());

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, EndingLevel);
}

void UETLevelSubsystem::TravelToCheckpoint(const FName InCheckpointRowName)
{
	const FETCheckpointTableRowData* CheckpointRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETCheckpointTableRowData>(InCheckpointRowName);
	if (ensureMsgf(CheckpointRowData, TEXT("Invalid CheckpointRowName: %s"), *InCheckpointRowName.ToString()) == false)
	{
		return;
	}

	UE_LOG(LogLevelTravel, Log, TEXT("TravelToCheckpoint: %s -> %s"), *InCheckpointRowName.ToString(), *CheckpointRowData->LevelRowName.ToString());

	UETSaveSubsystem& SaveSubsystem = ETSubsystem::Get<UETSaveSubsystem>(this);
	SaveSubsystem.GetWorldSaveData().CheckpointRowName = InCheckpointRowName;
	SaveSubsystem.Save();

	OpenLevel(CheckpointRowData->LevelRowName);
}

FName UETLevelSubsystem::FindLevelRowName(const UWorld* InWorld) const
{
	const UDataTable* LevelDataTable = ETSubsystem::Get<UETGameDataSubsystem>(this).GetDataTable<FETLevelTableRowData>();
	if (InWorld == nullptr || LevelDataTable == nullptr)
	{
		return NAME_None;
	}

	// PIE 에서는 패키지 이름에 접두어가 붙으므로 제거 후 비교
	const FString WorldPackageName = UWorld::RemovePIEPrefix(InWorld->GetOutermost()->GetName());

	for (const TPair<FName, uint8*>& RowPair : LevelDataTable->GetRowMap())
	{
		const FETLevelTableRowData* LevelRowData = reinterpret_cast<const FETLevelTableRowData*>(RowPair.Value);
		if (LevelRowData && LevelRowData->Level.ToSoftObjectPath().GetLongPackageName() == WorldPackageName)
		{
			return RowPair.Key;
		}
	}

	return NAME_None;
}

void UETLevelSubsystem::RecordCurrentLevel(const UWorld* InWorld)
{
	// 타이틀 등 테이블에 없는 레벨은 기록하지 않음
	const FName LevelRowName = FindLevelRowName(InWorld);
	UE_LOG(LogLevelTravel, Log, TEXT("RecordCurrentLevel: %s -> %s"), InWorld ? *InWorld->GetName() : TEXT("None"), *LevelRowName.ToString());

	if (LevelRowName.IsNone())
	{
		return;
	}

	UETSaveSubsystem& SaveSubsystem = ETSubsystem::Get<UETSaveSubsystem>(this);
	if (SaveSubsystem.GetWorldSaveData().LevelRowName == LevelRowName)
	{
		return;
	}

	SaveSubsystem.GetWorldSaveData().LevelRowName = LevelRowName;
	SaveSubsystem.Save();
}

void UETLevelSubsystem::RecordCharacterDefeat(const FName InCharacterRowName)
{
	UETSaveSubsystem& SaveSubsystem = ETSubsystem::Get<UETSaveSubsystem>(this);

	bool bAlreadyDefeated = false;
	SaveSubsystem.GetWorldSaveData().DefeatedCharacterRowNameSet.Add(InCharacterRowName, &bAlreadyDefeated);

	if (bAlreadyDefeated == false)
	{
		SaveSubsystem.Save();
	}
}

bool UETLevelSubsystem::IsCharacterDefeated(const FName InCharacterRowName) const
{
	return ETSubsystem::Get<UETSaveSubsystem>(this).GetWorldSaveData().DefeatedCharacterRowNameSet.Contains(InCharacterRowName);
}

void UETLevelSubsystem::TryClearLevel(const UWorld* InWorld, const FName InDefeatedCharacterRowName)
{
	const FName LevelRowName = FindLevelRowName(InWorld);
	const FETLevelTableRowData* LevelRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETLevelTableRowData>(LevelRowName);
	if (LevelRowData == nullptr || LevelRowData->ClearCharacterRowName.IsNone() || LevelRowData->ClearCharacterRowName != InDefeatedCharacterRowName)
	{
		return;
	}

	UETSaveSubsystem& SaveSubsystem = ETSubsystem::Get<UETSaveSubsystem>(this);

	bool bAlreadyCleared = false;
	SaveSubsystem.GetWorldSaveData().ClearedLevelRowNameSet.Add(LevelRowName, &bAlreadyCleared);

	if (bAlreadyCleared == false)
	{
		// 익사 등 처치자가 없어도 레벨 클리어 보상은 플레이어에게 (클리어 저장에 보상이 포함되도록 보상 적용 후 저장)
		if (AETCharacter* PlayerCharacter = Cast<AETCharacter>(UGameplayStatics::GetPlayerCharacter(InWorld, 0)))
		{
			PlayerCharacter->ApplyReward(LevelRowData->ClearRewardRowName);
		}

		SaveSubsystem.Save();
		OnLevelCleared.Broadcast(LevelRowName);
	}
}

bool UETLevelSubsystem::IsLevelCleared(const FName InLevelRowName) const
{
	return ETSubsystem::Get<UETSaveSubsystem>(this).GetWorldSaveData().ClearedLevelRowNameSet.Contains(InLevelRowName);
}
