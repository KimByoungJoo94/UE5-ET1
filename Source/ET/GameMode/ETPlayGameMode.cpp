// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ETPlayGameMode.h"
#include "Interaction/ETCheckpoint.h"
#include "Subsystem/ETSaveSubsystem.h"
#include "Subsystem/ETLevelSubsystem.h"
#include "EngineUtils.h"

void AETPlayGameMode::BeginPlay()
{
	Super::BeginPlay();

	// 들어온 경로(포탈, 체크포인트 이동, 이어하기)와 상관없이 레벨이 열리면 기록
	ETSubsystem::Get<UETLevelSubsystem>(this).RecordCurrentLevel(GetWorld());
}

void AETPlayGameMode::RestartPlayer(AController* NewPlayer)
{
	// PlayerStart 대신 체크포인트의 부활 위치를 쓰기 위해 Transform 지정 (컨트롤 회전도 이 값으로 설정됨)
	if (const AETCheckpoint* LastCheckpoint = FindLastCheckpoint())
	{
		RestartPlayerAtTransform(NewPlayer, LastCheckpoint->GetRespawnTransform());
		return;
	}

	Super::RestartPlayer(NewPlayer);
}

AETCheckpoint* AETPlayGameMode::FindLastCheckpoint() const
{
	const FName LastCheckpointRowName = ETSubsystem::Get<UETSaveSubsystem>(this).GetWorldSaveData().CheckpointRowName;
	if (LastCheckpointRowName.IsNone())
	{
		return nullptr;
	}

	for (TActorIterator<AETCheckpoint> It(GetWorld()); It; ++It)
	{
		if (It->GetCheckpointRowName() == LastCheckpointRowName)
		{
			return *It;
		}
	}

	return nullptr;
}
