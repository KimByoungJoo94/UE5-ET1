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

// 카메라 쉐이크 테이블 행으로 로컬 플레이어 카메라 흔들기 (시작 즉시 완료)
// 진원지를 넘기면 테이블 Inner, Outer 반경 기준으로 거리 감쇠, 아니면 그대로 재생
struct ET_API FETCameraShakeJob : public FETJob
{
public:
	FETCameraShakeJob(const FName InCameraShakeRowName);
	FETCameraShakeJob(const FName InCameraShakeRowName, const FVector& InEpicenter);

	virtual void Start(UWorld* InWorld) override;
	virtual EETJobStatus Tick(const float InDeltaTime) override;
	virtual void End(const bool bInCancelled) override;

private:
	FName CameraShakeRowName;
	TOptional<FVector> Epicenter;
};
