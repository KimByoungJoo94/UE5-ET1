#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Subsystem/ETSubsystem.h"
#include "ETSpawnSubsystem.generated.h"

class AETSpawnVolume;
class AETPlayer;

// 레벨의 스폰 볼륨 관리 (볼륨이 BeginPlay, EndPlay 에서 등록, 해제)
UCLASS()
class ET_API UETSpawnSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

public:
	void RegisterSpawnVolume(AETSpawnVolume* InSpawnVolume);
	void UnregisterSpawnVolume(AETSpawnVolume* InSpawnVolume);

	// 모든 볼륨의 몬스터를 처음 상태로 다시 생성 (체크포인트 휴식 등)
	void ResetAllSpawnVolumes();

	// 순간이동 목적지 후보 볼륨 중 랜덤으로 하나와 그 도착 위치 (후보가 없거나 모든 후보에서 위치를 못 찾으면 nullptr)
	AETSpawnVolume* FindTeleportDestination(const AETPlayer* InPlayer, OUT FTransform& OutTransform) const;

private:
	TArray<TWeakObjectPtr<AETSpawnVolume>> SpawnVolumeArray;
};
