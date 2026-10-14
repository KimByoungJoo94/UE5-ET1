#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "Subsystem/ETSubsystem.h"
#include "Job/ETJob.h"
#include "ETJobSubsystem.generated.h"

// Run 으로 실행한 Job 식별 (Cancel 용)
struct FETJobHandle
{
public:
	bool IsValid() const { return Id != 0; }

	bool operator==(const FETJobHandle& InOther) const { return Id == InOther.Id; }

private:
	friend class UETJobSubsystem;

	uint32 Id = 0;
};

// 액터, 위젯 등 어디서든 Job 을 실행하고 매 프레임 Tick (월드 DeltaTime 기준, 액터 CustomTimeDilation 은 미적용)
UCLASS()
class ET_API UETJobSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return RunningJobArray.IsEmpty() == false; }

public:
	// 바로 Start 하고 다음 프레임부터 Tick, InOwner 가 사라지면 자동 취소
	FETJobHandle Run(const UObject* InOwner, TUniquePtr<FETJob> InJob);

	void Cancel(const FETJobHandle& InHandle);
	void CancelAll(const UObject* InOwner);

private:
	struct FETRunningJob
	{
		FETJobHandle Handle;
		TWeakObjectPtr<const UObject> Owner;
		TUniquePtr<FETJob> Job;

		bool bFinished = false;
		bool bCancelled = false;
	};

	// 끝났거나 취소된 Job 을 목록에서 빼고 취소된 Job 은 End(true) 호출
	void RemoveEndedJobs();

private:
	TArray<FETRunningJob> RunningJobArray;

	uint32 LastHandleId = 0;

	// Tick 중에는 목록을 직접 지우지 않고 표시만 (Job 안에서 Run / Cancel 호출 가능)
	bool bTicking = false;
};
