#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETSpawnVolumeTableRowData.generated.h"

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
};
