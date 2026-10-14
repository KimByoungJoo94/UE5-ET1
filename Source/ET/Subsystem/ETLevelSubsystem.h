#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystem/ETSubsystem.h"
#include "ETLevelSubsystem.generated.h"

// 레벨을 다시 열어도 유지되는 진행 정보 (마지막 체크포인트 등) 및 레벨 이동
UCLASS()
class ET_API UETLevelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 현재 레벨을 다시 열어 레벨 상태 전체를 초기화 (스폰 위치는 GameMode 가 마지막 체크포인트 기준으로 결정)
	void RestartLevel();

	FORCEINLINE void SetLastCheckpointRowName(const FName InCheckpointRowName) { LastCheckpointRowName = InCheckpointRowName; }
	FORCEINLINE FName GetLastCheckpointRowName() const { return LastCheckpointRowName; }

private:
	// None 이면 기본 PlayerStart 에서 시작
	FName LastCheckpointRowName = NAME_None;
};
