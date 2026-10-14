#include "Subsystem/ETJobSubsystem.h"

void UETJobSubsystem::Deinitialize()
{
	for (FETRunningJob& RunningJob : RunningJobArray)
	{
		RunningJob.bCancelled = true;
	}

	RemoveEndedJobs();

	Super::Deinitialize();
}

void UETJobSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	bTicking = true;

	// 이번 프레임에 Run 된 Job 은 다음 프레임부터 Tick
	// Job 안에서 Run 하면 배열이 재할당될 수 있으므로 원소 참조를 Tick 호출 너머로 유지하지 않음
	const int32 RunningJobNum = RunningJobArray.Num();
	for (int32 Index = 0; Index < RunningJobNum; ++Index)
	{
		if (RunningJobArray[Index].bCancelled)
		{
			continue;
		}

		if (RunningJobArray[Index].Owner.IsValid() == false)
		{
			RunningJobArray[Index].bCancelled = true;
			continue;
		}

		FETJob* Job = RunningJobArray[Index].Job.Get();
		if (Job->Tick(DeltaTime) == EETJobStatus::Finished)
		{
			Job->End(false);
			RunningJobArray[Index].bFinished = true;
		}
	}

	bTicking = false;

	RemoveEndedJobs();
}

TStatId UETJobSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UETJobSubsystem, STATGROUP_Tickables);
}

FETJobHandle UETJobSubsystem::Run(const UObject* InOwner, TUniquePtr<FETJob> InJob)
{
	if (ensureMsgf(InOwner && InJob, TEXT("UETJobSubsystem::Run requires Owner and Job.")) == false)
	{
		return FETJobHandle();
	}

	FETRunningJob& RunningJob = RunningJobArray.AddDefaulted_GetRef();
	RunningJob.Handle.Id = ++LastHandleId;
	RunningJob.Owner = InOwner;
	RunningJob.Job = MoveTemp(InJob);

	const FETJobHandle Handle = RunningJob.Handle;

	// Start 안에서 Run 이 호출되면 배열이 재할당될 수 있으므로 Job 포인터로 호출
	FETJob* Job = RunningJob.Job.Get();
	Job->Start(GetWorld());

	return Handle;
}

void UETJobSubsystem::Cancel(const FETJobHandle& InHandle)
{
	FETRunningJob* RunningJob = RunningJobArray.FindByPredicate([&InHandle](const FETRunningJob& InRunningJob)
	{
		return InRunningJob.Handle == InHandle;
	});

	if (RunningJob == nullptr || RunningJob->bFinished)
	{
		return;
	}

	RunningJob->bCancelled = true;

	if (bTicking == false)
	{
		RemoveEndedJobs();
	}
}

void UETJobSubsystem::CancelAll(const UObject* InOwner)
{
	for (FETRunningJob& RunningJob : RunningJobArray)
	{
		if (RunningJob.Owner.Get() == InOwner && RunningJob.bFinished == false)
		{
			RunningJob.bCancelled = true;
		}
	}

	if (bTicking == false)
	{
		RemoveEndedJobs();
	}
}

void UETJobSubsystem::RemoveEndedJobs()
{
	// 먼저 목록에서 빼낸 뒤 End 호출 (End 안에서 Run / Cancel 이 호출되어도 목록이 꼬이지 않도록)
	TArray<FETRunningJob> CancelledJobArray;

	for (int32 Index = RunningJobArray.Num() - 1; Index >= 0; --Index)
	{
		if (RunningJobArray[Index].bFinished)
		{
			RunningJobArray.RemoveAt(Index);
		}
		else if (RunningJobArray[Index].bCancelled)
		{
			CancelledJobArray.Add(MoveTemp(RunningJobArray[Index]));
			RunningJobArray.RemoveAt(Index);
		}
	}

	for (FETRunningJob& CancelledJob : CancelledJobArray)
	{
		CancelledJob.Job->End(true);
	}
}
