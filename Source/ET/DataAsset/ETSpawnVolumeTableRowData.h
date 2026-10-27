#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETSpawnVolumeTableRowData.generated.h"

class UMaterialInterface;

// 스폰 볼륨이 생성할 캐릭터 하나 (위치, 회전은 볼륨 기준 상대값, 높이는 생성 시 바닥에 맞춤)
USTRUCT()
struct ET_API FETSpawnVolumeCharacterData
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditAnywhere)
	bool bSpawnEnabled = true;

	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetCharacterRowNameOptions"))
	FName CharacterRowName;

	UPROPERTY(EditAnywhere)
	FVector RelativeLocation = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FRotator RelativeRotation = FRotator::ZeroRotator;
};

// 스폰 볼륨 구성 (위치는 레벨에 배치한 AETSpawnVolume 이 가짐)
USTRUCT()
struct ET_API FETSpawnVolumeTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 볼륨 중심 기준 범위, 생성한 몬스터의 대상 인식, 유지 판정에 사용
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float Range = 1500.f;

	UPROPERTY(EditAnywhere)
	TArray<FETSpawnVolumeCharacterData> SpawnCharacterArray;

	// 플레이어가 범위 안쪽으로 들어오면 생성한 몬스터를 모두 처치할 때까지 범위 밖으로 나갈 수 없음 (수평 거리 기준)
	UPROPERTY(EditAnywhere)
	bool bLockPlayerInRange = false;

	// 경계에서 이 거리만큼 더 안쪽으로 들어와야 잠금 시작 (들어오자마자 경계에 막히지 않도록, 잠금 후 막히는 위치는 경계 그대로)
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bLockPlayerInRange", ClampMin = "0.0"))
	float LockEnterMargin = 200.f;

	// 잠금 중 범위 경계(구)에 표시할 머티리얼 (안쪽에서도 보이도록 양면 머티리얼, 비어 있으면 표시 없음)
	UPROPERTY(EditAnywhere, meta = (EditCondition = "bLockPlayerInRange"))
	TSoftObjectPtr<UMaterialInterface> LockBoundaryMaterial;
};
