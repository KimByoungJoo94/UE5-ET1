#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "Subsystem/ETSubsystem.h"
#include "ETLevelSubsystem.generated.h"

// 레벨 이동 (진행 정보는 UETSaveSubsystem 이 보관)
UCLASS()
class ET_API UETLevelSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	// 현재 레벨을 다시 열어 레벨 상태 전체를 초기화 (스폰 위치는 GameMode 가 저장된 체크포인트 기준으로 결정)
	void RestartLevel();
};
