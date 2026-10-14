#pragma once

#include "CoreMinimal.h"
#include "Job/ETJob.h"

// 현재 레벨 재시작 (마지막 체크포인트 또는 기본 PlayerStart 에서 시작)
struct ET_API FETRestartLevelJob : public FETJob
{
public:
	virtual void Start(UWorld* InWorld) override;
	virtual EETJobStatus Tick(const float InDeltaTime) override;
	virtual void End(const bool bInCancelled) override;
};
