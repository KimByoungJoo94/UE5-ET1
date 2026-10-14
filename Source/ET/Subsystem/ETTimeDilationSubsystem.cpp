#include "Subsystem/ETTimeDilationSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UETTimeDilationSubsystem::Deinitialize()
{
	StopGlobalTimeDilation();
	ActorRequestMap.Reset();

	Super::Deinitialize();
}

void UETTimeDilationSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	const double RealTimeSeconds = GetWorld()->GetRealTimeSeconds();

	if (bGlobalTimeDilationActive && RealTimeSeconds >= GlobalEndRealTimeSeconds)
	{
		StopGlobalTimeDilation();
	}

	TArray<TWeakObjectPtr<AActor>> ChangedActorArray;
	for (auto Iter = ActorRequestMap.CreateIterator(); Iter; ++Iter)
	{
		if (Iter->Key.IsValid() == false)
		{
			Iter.RemoveCurrent();
			continue;
		}

		const int32 RemovedNum = Iter->Value.RemoveAll([RealTimeSeconds](const FETActorTimeDilationRequest& InRequest)
		{
			return InRequest.EndRealTimeSeconds > 0.0 && RealTimeSeconds >= InRequest.EndRealTimeSeconds;
		});

		if (RemovedNum > 0)
		{
			ChangedActorArray.Add(Iter->Key);
		}
	}

	// 순회 중 맵 변경을 피하려고 따로 적용
	for (const TWeakObjectPtr<AActor>& ChangedActor : ChangedActorArray)
	{
		ApplyActorTimeDilation(ChangedActor);
	}
}

TStatId UETTimeDilationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UETTimeDilationSubsystem, STATGROUP_Tickables);
}

void UETTimeDilationSubsystem::StartGlobalTimeDilation(const float InTimeDilation, const float InRealDuration)
{
	UGameplayStatics::SetGlobalTimeDilation(this, InTimeDilation);

	bGlobalTimeDilationActive = true;
	GlobalEndRealTimeSeconds = GetWorld()->GetRealTimeSeconds() + InRealDuration;
}

void UETTimeDilationSubsystem::StopGlobalTimeDilation()
{
	if (bGlobalTimeDilationActive == false)
	{
		return;
	}

	bGlobalTimeDilationActive = false;
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
}

void UETTimeDilationSubsystem::AddActorTimeDilation(AActor* InActor, const EETTimeDilationReason InReason, const float InTimeDilation, const float InRealDuration)
{
	if (IsValid(InActor) == false)
	{
		return;
	}

	TArray<FETActorTimeDilationRequest>& RequestArray = ActorRequestMap.FindOrAdd(InActor);

	FETActorTimeDilationRequest* Request = RequestArray.FindByPredicate([InReason](const FETActorTimeDilationRequest& InRequest)
	{
		return InRequest.Reason == InReason;
	});

	if (Request == nullptr)
	{
		Request = &RequestArray.AddDefaulted_GetRef();
		Request->Reason = InReason;
	}

	Request->TimeDilation = InTimeDilation;
	Request->EndRealTimeSeconds = InRealDuration > 0.f ? GetWorld()->GetRealTimeSeconds() + InRealDuration : 0.0;

	ApplyActorTimeDilation(InActor);
}

void UETTimeDilationSubsystem::RemoveActorTimeDilation(AActor* InActor, const EETTimeDilationReason InReason)
{
	TArray<FETActorTimeDilationRequest>* RequestArray = ActorRequestMap.Find(InActor);
	if (RequestArray == nullptr)
	{
		return;
	}

	const int32 RemovedNum = RequestArray->RemoveAll([InReason](const FETActorTimeDilationRequest& InRequest)
	{
		return InRequest.Reason == InReason;
	});

	if (RemovedNum > 0)
	{
		ApplyActorTimeDilation(InActor);
	}
}

void UETTimeDilationSubsystem::ApplyActorTimeDilation(const TWeakObjectPtr<AActor>& InActor)
{
	const TArray<FETActorTimeDilationRequest>* RequestArray = ActorRequestMap.Find(InActor);

	AActor* Actor = InActor.Get();
	if (Actor == nullptr)
	{
		ActorRequestMap.Remove(InActor);
		return;
	}

	if (RequestArray == nullptr || RequestArray->IsEmpty())
	{
		Actor->CustomTimeDilation = 1.f;
		ActorRequestMap.Remove(InActor);
		return;
	}

	// 가장 느린 요청 적용 (시간 정지 중 히트 스톱이 와도 정지 유지)
	float TimeDilation = 1.f;
	for (const FETActorTimeDilationRequest& Request : *RequestArray)
	{
		TimeDilation = FMath::Min(TimeDilation, Request.TimeDilation);
	}

	Actor->CustomTimeDilation = TimeDilation;
}
