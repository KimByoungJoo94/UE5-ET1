#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "DataAsset/ETGameStatTableRowData.h"
#include "ETSaveGame.generated.h"

// 월드 진행 (체크포인트 등)
USTRUCT()
struct ET_API FETWorldSaveData
{
	GENERATED_BODY()

public:
	// 마지막으로 있던 레벨 (이어하기 시 열 레벨, 포탈, 체크포인트 이동, 휴식 시 갱신)
	UPROPERTY()
	FName LevelRowName = NAME_None;

	// 마지막으로 쉰 체크포인트, 현재 레벨에 없거나 None 이면 기본 PlayerStart
	UPROPERTY()
	FName CheckpointRowName = NAME_None;

	// 상호작용한 적 있는 체크포인트 (체크포인트 이동 목록)
	UPROPERTY()
	TSet<FName> DiscoveredCheckpointRowNameSet;

	// 한 번이라도 처치한 캐릭터 RowName (보스 또는 1회 보상이 있는 캐릭터만, 1회 보상 지급, 레벨 클리어 판정)
	UPROPERTY()
	TSet<FName> DefeatedCharacterRowNameSet;
};

// 플레이어 진행 (영구 스탯 등)
USTRUCT()
struct ET_API FETPlayerSaveData
{
	GENERATED_BODY()

public:
	// 스탯 종류 -> FETGameStat::PermanentValue
	UPROPERTY()
	TMap<EETGameStatType, float> PermanentGameStatMap;
};

// 한 회차의 진행 데이터 (슬롯 하나 = 한 회차), 범주별 구조체로 확장
UCLASS()
class ET_API UETSaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	// 저장 구조가 크게 바뀌면 올리고 로드 시 이전 버전 데이터를 변환
	static constexpr int32 LatestSaveVersion = 1;

	UPROPERTY()
	int32 SaveVersion = LatestSaveVersion;

	UPROPERTY()
	FETWorldSaveData WorldSaveData;

	UPROPERTY()
	FETPlayerSaveData PlayerSaveData;
};
