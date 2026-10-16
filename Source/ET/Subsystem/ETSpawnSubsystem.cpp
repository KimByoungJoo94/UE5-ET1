#include "Subsystem/ETSpawnSubsystem.h"
#include "Spawn/ETSpawnVolume.h"

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
