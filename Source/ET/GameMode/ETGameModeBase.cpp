// Fill out your copyright notice in the Description page of Project Settings.


#include "GameMode/ETGameModeBase.h"
#include "Interaction/ETCheckpoint.h"
#include "Subsystem/ETLevelSubsystem.h"
#include "EngineUtils.h"

void AETGameModeBase::RestartPlayer(AController* NewPlayer)
{
	// PlayerStart 대신 체크포인트의 부활 위치를 쓰기 위해 Transform 지정 (컨트롤 회전도 이 값으로 설정됨)
	if (const AETCheckpoint* LastCheckpoint = FindLastCheckpoint())
	{
		RestartPlayerAtTransform(NewPlayer, LastCheckpoint->GetRespawnTransform());
		return;
	}

	Super::RestartPlayer(NewPlayer);
}

AETCheckpoint* AETGameModeBase::FindLastCheckpoint() const
{
	const FName LastCheckpointRowName = ETSubsystem::Get<UETLevelSubsystem>(this).GetLastCheckpointRowName();
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
