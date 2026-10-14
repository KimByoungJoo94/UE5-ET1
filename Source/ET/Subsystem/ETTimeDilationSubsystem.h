#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Subsystem/ETSubsystem.h"
#include "ETTimeDilationSubsystem.generated.h"

// 액터별 감속 요청 사유 (같은 사유는 하나만 유지, 다시 요청하면 갱신)
UENUM()
enum class EETTimeDilationReason : uint8
{
	TimeFreeze,
	HitStop,
};

struct FETActorTimeDilationRequest
{
	EETTimeDilationReason Reason = EETTimeDilationReason::TimeFreeze;
	float TimeDilation = 1.f;

	// 0 이하면 Remove 전까지 유지
	double EndRealTimeSeconds = 0.0;
};

// 전체 시간 감속(GlobalTimeDilation)과 액터별 감속(CustomTimeDilation)을 한곳에서 관리
// 액터는 남은 요청 중 가장 느린 값을 적용, 요청이 없으면 1.0 으로 복구
UCLASS()
class ET_API UETTimeDilationSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return bGlobalTimeDilationActive || ActorRequestMap.IsEmpty() == false; }

public:
	void StartGlobalTimeDilation(const float InTimeDilation, const float InRealDuration);
	void StopGlobalTimeDilation();

	// InRealDuration 이 0 이하면 RemoveActorTimeDilation 전까지 유지
	void AddActorTimeDilation(AActor* InActor, const EETTimeDilationReason InReason, const float InTimeDilation, const float InRealDuration = 0.f);
	void RemoveActorTimeDilation(AActor* InActor, const EETTimeDilationReason InReason);

private:
	void ApplyActorTimeDilation(const TWeakObjectPtr<AActor>& InActor);

private:
	bool bGlobalTimeDilationActive = false;
	double GlobalEndRealTimeSeconds = 0.0;

	TMap<TWeakObjectPtr<AActor>, TArray<FETActorTimeDilationRequest>> ActorRequestMap;
};
