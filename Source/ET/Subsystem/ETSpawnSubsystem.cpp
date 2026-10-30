#include "Subsystem/ETSpawnSubsystem.h"
#include "Spawn/ETSpawnVolume.h"
#include "Algo/RandomShuffle.h"

bool UETSpawnSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

void UETSpawnSubsystem::RegisterSpawnVolume(AETSpawnVolume* InSpawnVolume)
{
	SpawnVolumeArray.AddUnique(InSpawnVolume);
}

void UETSpawnSubsystem::UnregisterSpawnVolume(AETSpawnVolume* InSpawnVolume)
{
	SpawnVolumeArray.Remove(InSpawnVolume);
}

void UETSpawnSubsystem::ResetAllSpawnVolumes()
{
	for (const TWeakObjectPtr<AETSpawnVolume>& SpawnVolume : SpawnVolumeArray)
	{
		if (SpawnVolume.IsValid())
		{
			SpawnVolume->ResetSpawn();
		}
	}
}

AETSpawnVolume* UETSpawnSubsystem::FindTeleportDestination(const AETPlayer* InPlayer, OUT FTransform& OutTransform) const
{
	TArray<AETSpawnVolume*> CandidateArray;
	for (const TWeakObjectPtr<AETSpawnVolume>& SpawnVolume : SpawnVolumeArray)
	{
		if (SpawnVolume.IsValid() && SpawnVolume->IsTeleportDestination(InPlayer))
		{
			CandidateArray.Add(SpawnVolume.Get());
		}
	}

	// 랜덤 순서로 도착 위치를 찾을 수 있는 첫 볼륨
	Algo::RandomShuffle(CandidateArray);

	AETSpawnVolume* DestinationVolume = nullptr;
	for (AETSpawnVolume* Candidate : CandidateArray)
	{
		if (Candidate->FindTeleportTransform(InPlayer, OutTransform))
		{
			DestinationVolume = Candidate;
			break;
		}
	}

	return DestinationVolume;
}
