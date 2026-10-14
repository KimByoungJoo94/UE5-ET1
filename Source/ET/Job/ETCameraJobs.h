#pragma once

#include "CoreMinimal.h"
#include "Job/ETJob.h"

class USpringArmComponent;

// 스프링암 길이를 목표 길이까지 보간 (줌 인 / 아웃)
struct ET_API FETCameraZoomJob : public FETJob
{
public:
	FETCameraZoomJob(USpringArmComponent* InSpringArmComponent, const float InTargetArmLength = 1000.f, const float InDuration = 3.f);

	virtual void Start(UWorld* InWorld) override;
	virtual EETJobStatus Tick(const float InDeltaTime) override;
	virtual void End(const bool bInCancelled) override;

private:
	TWeakObjectPtr<USpringArmComponent> SpringArmComponent;

	float TargetArmLength = 0.f;
	float Duration = 0.f;

	float StartArmLength = 0.f;
	float ElapsedTime = 0.f;
};
