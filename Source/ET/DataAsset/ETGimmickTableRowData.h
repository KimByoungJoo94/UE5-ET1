#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETGimmickTableRowData.generated.h"

class UParticleSystem;

// 상호작용 등으로 발동하는 기믹 (각 효과는 비워 두거나 끄면 없음, 폭발 → 스폰 볼륨 → 순간이동 순서)
USTRUCT()
struct ET_API FETGimmickTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 발동까지 유예 (0 이면 즉시)
	UPROPERTY(EditAnywhere, Category = "Common", meta = (ClampMin = "0.0"))
	float Delay = 0.f;

	// 기믹 시작 시 기믹 중심 + 로컬 오프셋 위치에 출력 (유예가 0 이어도 출력, 루프 파티클은 일정 시간 뒤 정지)
	UPROPERTY(EditAnywhere, Category = "Common")
	TSoftObjectPtr<UParticleSystem> WarningParticle;

	// 기믹 중심 기준 (X 전방, Y 우측, Z 위)
	UPROPERTY(EditAnywhere, Category = "Common")
	FVector WarningParticleLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere, Category = "Common", meta = (ClampMin = "0.0"))
	float WarningParticleScale = 1.f;

	// 기믹 중심에서 같은 편 판정 없이 범위 안 모든 캐릭터에게 데미지
	UPROPERTY(EditAnywhere, Category = "Explosion", meta = (GetOptions = "ET.ETGameDataSubsystem.GetRadialAttackCollisionRowNameOptions"))
	FName ExplosionRadialAttackCollisionRowName;

	UPROPERTY(EditAnywhere, Category = "Explosion", meta = (ClampMin = "0.0"))
	float ExplosionDamage = 0.f;

	// 기믹 위치, 방향으로 스폰 볼륨 생성 후 생성된 몬스터가 발동시킨 캐릭터를 바로 인식 (등장 연출 재생)
	// 휴식 때 다시 나오지 않게 하려면 볼륨 행의 bRespawnOnReset 을 끔
	UPROPERTY(EditAnywhere, Category = "SpawnVolume", meta = (GetOptions = "ET.ETGameDataSubsystem.GetSpawnVolumeRowNameOptions"))
	FName SpawnVolumeRowName;

	// 발동시킨 플레이어를 레벨의 스폰 볼륨 중 하나(살아 있는 몬스터가 있고 보스가 없으며 플레이어가 범위 밖)로 순간이동
	// 볼륨 범위 안쪽 잠금 시작 거리 근처에서 볼륨 중심을 바라보며 도착, 그 볼륨 몬스터가 바로 인식 (후보가 없으면 순간이동 없음)
	UPROPERTY(EditAnywhere, Category = "Teleport")
	bool bTeleport = false;
};
