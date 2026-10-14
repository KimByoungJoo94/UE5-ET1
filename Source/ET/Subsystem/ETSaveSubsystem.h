#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystem/ETSubsystem.h"
#include "Save/ETSaveGame.h"
#include "ETSaveSubsystem.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnSaveStarted);
DECLARE_MULTICAST_DELEGATE(FOnSaveFinished);

// 회차 진행 데이터의 원본 (게임 중에는 메모리의 데이터를 읽고 쓰고, Save 호출 시 슬롯에 기록)
// GameInstance 초기화 시 첫 레벨 로드 전에 슬롯에서 불러옴
UCLASS()
class ET_API UETSaveSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;

	// 게임 종료 시 동기 저장 (비동기 저장은 완료 전에 종료될 수 있음)
	virtual void Deinitialize() override;

public:
	FETWorldSaveData& GetWorldSaveData() { return SaveGame->WorldSaveData; }
	const FETWorldSaveData& GetWorldSaveData() const { return SaveGame->WorldSaveData; }

	FETPlayerSaveData& GetPlayerSaveData() { return SaveGame->PlayerSaveData; }
	const FETPlayerSaveData& GetPlayerSaveData() const { return SaveGame->PlayerSaveData; }

	// 현재 데이터를 슬롯에 비동기 저장 (저장 중에 다시 호출되면 끝난 뒤 마지막 데이터로 한 번 더 저장)
	void Save();

	bool HasSaveGameInSlot() const;

	// 새 게임 / 테스트용 (메모리 데이터를 기본값으로 되돌리고 슬롯 파일 삭제)
	void ResetSaveGame();

public:
	// 비동기 저장 시작 / 모든 저장 완료 (게임 종료 시 동기 저장은 알리지 않음)
	FOnSaveStarted OnSaveStarted;
	FOnSaveFinished OnSaveFinished;

private:
	void StartAsyncSave();
	void OnAsyncSaveFinished(const FString& InSlotName, const int32 InUserIndex, bool bInSuccess);

private:
	UPROPERTY(Transient)
	TObjectPtr<UETSaveGame> SaveGame;

	bool bSaving = false;
	bool bSavePending = false;
	bool bDeletePending = false;
};
