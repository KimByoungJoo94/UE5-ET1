#include "Subsystem/ETLevelSubsystem.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Subsystem/ETSaveSubsystem.h"
#include "DataAsset/ETLevelTableRowData.h"
#include "DataAsset/ETCheckpointTableRowData.h"
#include "Common/ETCommonSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TimerManager.h"

void UETLevelSubsystem::RestartLevel()
{
	UGameplayStatics::OpenLevel(this, FName(UGameplayStatics::GetCurrentLevelName(this)));
}

void UETLevelSubsystem::OpenLevel(const FName InLevelRowName)
{
	const FETLevelTableRowData* LevelRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETLevelTableRowData>(InLevelRowName);
	if (ensureMsgf(LevelRowData && LevelRowData->Level.IsNull() == false, TEXT("Invalid LevelRowName: %s"), *InLevelRowName.ToString()) == false)
	{
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, LevelRowData->Level);
}

void UETLevelSubsystem::OpenTitleLevel()
{
	const TSoftObjectPtr<UWorld>& TitleLevel = GetDefault<UETCommonSettings>()->Level.TitleLevel;
	if (ensureMsgf(TitleLevel.IsNull() == false, TEXT("TitleLevel is not set in ET Common Settings.")) == false)
	{
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, TitleLevel);
}

void UETLevelSubsystem::OpenEndingLevel()
{
	const TSoftObjectPtr<UWorld>& EndingLevel = GetDefault<UETCommonSettings>()->Level.EndingLevel;
	if (ensureMsgf(EndingLevel.IsNull() == false, TEXT("EndingLevel is not set in ET Common Settings.")) == false)
	{
		return;
	}

	UGameplayStatics::OpenLevelBySoftObjectPtr(this, EndingLevel);
}

void UETLevelSubsystem::TravelToCheckpoint(const FName InCheckpointRowName)
{
	const FETCheckpointTableRowData* CheckpointRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETCheckpointTableRowData>(InCheckpointRowName);
	if (ensureMsgf(CheckpointRowData, TEXT("Invalid CheckpointRowName: %s"), *InCheckpointRowName.ToString()) == false)
	{
		return;
	}

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

void UETLevelSubsystem::RecordCharacterDefeat(const FName InCharacterRowName, const bool bInBoss)
{
	UETSaveSubsystem& SaveSubsystem = ETSubsystem::Get<UETSaveSubsystem>(this);

	bool bAlreadyDefeated = false;
	SaveSubsystem.GetWorldSaveData().DefeatedCharacterRowNameSet.Add(InCharacterRowName, &bAlreadyDefeated);

	if (bAlreadyDefeated == false)
	{
		SaveSubsystem.Save();
	}

	if (bInBoss)
	{
		OnBossDefeated.Broadcast(InCharacterRowName);
		TryStartEnding(InCharacterRowName);
	}
}

bool UETLevelSubsystem::IsCharacterDefeated(const FName InCharacterRowName) const
{
	return ETSubsystem::Get<UETSaveSubsystem>(this).GetWorldSaveData().DefeatedCharacterRowNameSet.Contains(InCharacterRowName);
}

bool UETLevelSubsystem::IsLevelCleared(const FName InLevelRowName) const
{
	const FETLevelTableRowData* LevelRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETLevelTableRowData>(InLevelRowName);
	if (LevelRowData == nullptr || LevelRowData->ClearCharacterRowName.IsNone())
	{
		return false;
	}

	return IsCharacterDefeated(LevelRowData->ClearCharacterRowName);
}

void UETLevelSubsystem::TryStartEnding(const FName InCharacterRowName)
{
	const FName LevelRowName = FindLevelRowName(GetWorld());
	if (LevelRowName.IsNone())
	{
		return;
	}

	const FETLevelTableRowData* LevelRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETLevelTableRowData>(LevelRowName);
	if (LevelRowData == nullptr || LevelRowData->NextLevelRowName.IsNone() == false || LevelRowData->ClearCharacterRowName != InCharacterRowName)
	{
		return;
	}

	const float EndingDelay = GetDefault<UETCommonSettings>()->Level.EndingDelay;
	if (EndingDelay <= 0.f)
	{
		OpenEndingLevel();
		return;
	}

	// 월드 타이머라 그 전에 레벨이 바뀌면 함께 취소됨
	FTimerHandle EndingTimerHandle;
	GetWorld()->GetTimerManager().SetTimer(EndingTimerHandle, this, &ThisClass::OpenEndingLevel, EndingDelay, false);
}
