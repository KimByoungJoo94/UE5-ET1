#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystem/ETSubsystem.h"
#include "ETLevelSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnLevelCleared, FName InLevelRowName);

// 레벨 이동, 레벨 진행 판정 (진행 정보는 UETSaveSubsystem 이 보관)
UCLASS()
class ET_API UETLevelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 현재 레벨을 다시 열어 레벨 상태 전체를 초기화 (스폰 위치는 GameMode 가 저장된 체크포인트 기준으로 결정)
	void RestartLevel();

	// 레벨 테이블 행의 레벨 열기 (스폰 위치는 GameMode 가 결정)
	void OpenLevel(const FName InLevelRowName);

	// 메인 타이틀 레벨 열기 (엔딩 후, 타이틀로 돌아가기 등)
	void OpenTitleLevel();

	// 엔딩 레벨 열기 (마지막 레벨 클리어 후 포탈 상호작용 시)
	void OpenEndingLevel();

	// 체크포인트를 부활 위치로 저장하고 소속 레벨 열기 (같은 레벨이어도 다시 열어서 상태 초기화)
	void TravelToCheckpoint(const FName InCheckpointRowName);

	// 월드에 해당하는 레벨 테이블 RowName (테이블에 없는 레벨이면 None)
	FName FindLevelRowName(const UWorld* InWorld) const;

	// 레벨 시작 시 마지막 레벨로 기록 (이어하기 시 열 레벨)
	void RecordCurrentLevel(const UWorld* InWorld);

	// 처치 기록 (최초 처치일 때만 저장)
	void RecordCharacterDefeat(const FName InCharacterRowName);
	bool IsCharacterDefeated(const FName InCharacterRowName) const;

	// 처치한 캐릭터가 현재 레벨의 클리어 캐릭터면 레벨 클리어 기록 (최초 클리어면 플레이어에게 클리어 보상, OnLevelCleared 알림)
	void TryClearLevel(const UWorld* InWorld, const FName InDefeatedCharacterRowName);
	bool IsLevelCleared(const FName InLevelRowName) const;

public:
	FOnLevelCleared OnLevelCleared;
};
