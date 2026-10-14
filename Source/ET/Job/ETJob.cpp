#include "Job/ETJob.h"
#include "Engine/World.h"

void FETJobChain::Start(UWorld* InWorld)
{
	World = InWorld;
	CurrentIndex = 0;

	if (JobArray.IsValidIndex(CurrentIndex))
	{
		JobArray[CurrentIndex]->Start(InWorld);
	}
}

EETJobStatus FETJobChain::Tick(const float InDeltaTime)
{
	float DeltaTime = InDeltaTime;

	while (JobArray.IsValidIndex(CurrentIndex))
	{
		if (JobArray[CurrentIndex]->Tick(DeltaTime) == EETJobStatus::Running)
		{
			return EETJobStatus::Running;
		}

		JobArray[CurrentIndex]->End(false);

		// 다음 Job 은 같은 프레임에 시작하고, 이번 프레임 시간은 이미 앞 Job 이 사용했으므로 0 으로 Tick
		++CurrentIndex;
		DeltaTime = 0.f;

		if (JobArray.IsValidIndex(CurrentIndex))
		{
			JobArray[CurrentIndex]->Start(World.Get());
		}
	}

	return EETJobStatus::Finished;
}

void FETJobChain::End(const bool bInCancelled)
{
	// 진행 중이던 Job 만 중단 (끝난 Job 은 Tick 에서 이미 End 호출)
	if (bInCancelled && JobArray.IsValidIndex(CurrentIndex))
	{
		JobArray[CurrentIndex]->End(true);
	}

	CurrentIndex = INDEX_NONE;
}

void FETJobParallel::Start(UWorld* InWorld)
{
	FinishedArray.Init(false, JobArray.Num());

	for (const TUniquePtr<FETJob>& Job : JobArray)
	{
		Job->Start(InWorld);
	}
}

EETJobStatus FETJobParallel::Tick(const float InDeltaTime)
{
	bool bAllFinished = true;

	for (int32 Index = 0; Index < JobArray.Num(); ++Index)
	{
		if (FinishedArray[Index])
		{
			continue;
		}

		if (JobArray[Index]->Tick(InDeltaTime) == EETJobStatus::Finished)
		{
			JobArray[Index]->End(false);
			FinishedArray[Index] = true;
		}
		else
		{
			bAllFinished = false;
		}
	}

	return bAllFinished ? EETJobStatus::Finished : EETJobStatus::Running;
}

void FETJobParallel::End(const bool bInCancelled)
{
	if (bInCancelled == false)
	{
		return;
	}

	for (int32 Index = 0; Index < JobArray.Num(); ++Index)
	{
		if (FinishedArray.IsValidIndex(Index) && FinishedArray[Index] == false)
		{
			JobArray[Index]->End(true);
		}
	}
}
