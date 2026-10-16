#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Subsystem/ETSubsystem.h"
#include "ETSpawnSubsystem.generated.h"

class AETSpawnVolume;

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

private:
	TArray<TWeakObjectPtr<AETSpawnVolume>> SpawnVolumeArray;
};
