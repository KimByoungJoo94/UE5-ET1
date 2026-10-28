#include "Job/ETCameraJobs.h"
#include "DataAsset/ETCameraShakeTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Camera/CameraShakeBase.h"
#include "Camera/CameraModifier_CameraShake.h"
#include "Camera/PlayerCameraManager.h"
#include "GameFramework/PlayerController.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"

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

FETCameraShakeJob::FETCameraShakeJob(const FName InCameraShakeRowName)
	: CameraShakeRowName(InCameraShakeRowName)
{
}

FETCameraShakeJob::FETCameraShakeJob(const FName InCameraShakeRowName, const FVector& InEpicenter)
	: CameraShakeRowName(InCameraShakeRowName)
	, Epicenter(InEpicenter)
{
}

void FETCameraShakeJob::Start(UWorld* InWorld)
{
	const FETCameraShakeTableRowData* RowData = ETSubsystem::Get<UETGameDataSubsystem>(InWorld).GetRow<FETCameraShakeTableRowData>(CameraShakeRowName);
	const APlayerController* PlayerController = UGameplayStatics::GetPlayerController(InWorld, 0);
	APlayerCameraManager* PlayerCameraManager = PlayerController ? PlayerController->PlayerCameraManager : nullptr;
	if (RowData == nullptr || PlayerCameraManager == nullptr)
	{
		return;
	}

	const TSubclassOf<UCameraShakeBase> CameraShakeClass = RowData->CameraShakeClass.LoadSynchronous();
	if (CameraShakeClass == nullptr)
	{
		return;
	}

	float ShakeScale = RowData->Scale;
	if (Epicenter.IsSet())
	{
		// 카메라 위치 기준 Inner 안은 1, Outer 까지 선형 감쇠, Outer 밖은 0 (엔진 PlayWorldCameraShake 와 동일)
		const float Distance = FVector::Dist(Epicenter.GetValue(), PlayerCameraManager->GetCameraLocation());
		const float FalloffRange = RowData->OuterRadius - RowData->InnerRadius;
		const float DistanceScale = FalloffRange > 0.f ? 1.f - FMath::Clamp((Distance - RowData->InnerRadius) / FalloffRange, 0.f, 1.f) : (Distance < RowData->InnerRadius ? 1.f : 0.f);
		ShakeScale *= DistanceScale;
	}

	if (ShakeScale > 0.f)
	{
		FAddCameraShakeParams ShakeParams(ShakeScale);
		if (RowData->Duration > 0.f)
		{
			ShakeParams.DurationOverride = RowData->Duration;
		}

		PlayerCameraManager->StartCameraShake(CameraShakeClass, ShakeParams);
	}
}

EETJobStatus FETCameraShakeJob::Tick(const float InDeltaTime)
{
	return EETJobStatus::Finished;
}

void FETCameraShakeJob::End(const bool bInCancelled)
{
}
