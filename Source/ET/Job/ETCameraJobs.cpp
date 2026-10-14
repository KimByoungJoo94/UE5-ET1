#include "Job/ETCameraJobs.h"
#include "GameFramework/SpringArmComponent.h"

FETCameraZoomJob::FETCameraZoomJob(USpringArmComponent* InSpringArmComponent, const float InTargetArmLength, const float InDuration)
	: SpringArmComponent(InSpringArmComponent)
	, TargetArmLength(InTargetArmLength)
	, Duration(InDuration)
{
}

void FETCameraZoomJob::Start(UWorld* InWorld)
{
	if (SpringArmComponent.IsValid())
	{
		StartArmLength = SpringArmComponent->TargetArmLength;
	}

	ElapsedTime = 0.f;
}

EETJobStatus FETCameraZoomJob::Tick(const float InDeltaTime)
{
	if (SpringArmComponent.IsValid() == false)
	{
		return EETJobStatus::Finished;
	}

	ElapsedTime += InDeltaTime;

	const float Alpha = Duration > 0.f ? FMath::Clamp(ElapsedTime / Duration, 0.f, 1.f) : 1.f;
	SpringArmComponent->TargetArmLength = FMath::InterpEaseInOut(StartArmLength, TargetArmLength, Alpha, 2.f);

	return Alpha >= 1.f ? EETJobStatus::Finished : EETJobStatus::Running;
}

void FETCameraZoomJob::End(const bool bInCancelled)
{
	// 취소되면 현재 길이 그대로 유지
}
