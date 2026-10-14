#pragma once

#include "CoreMinimal.h"

class UWorld;

enum class EETJobStatus : uint8
{
	Running,
	Finished,
};

// 시간에 걸쳐 진행되는 작업 단위 (UETJobSubsystem 이 Start → Tick(Finished 반환까지) → End 순서로 호출)
struct ET_API FETJob
{
public:
	FETJob() = default;
	virtual ~FETJob() = default;

	// 자식 Job 을 TUniquePtr 로 소유하므로 복사 불가 (ET_API 로 export 시 암시적 복사 생성자가 만들어지지 않도록)
	UE_NONCOPYABLE(FETJob);

	virtual void Start(UWorld* InWorld) = 0;
	virtual EETJobStatus Tick(const float InDeltaTime) = 0;

	// bInCancelled : 완료 전에 중단됨 (Cancel, 소유자 소멸, 레벨 전환)
	virtual void End(const bool bInCancelled) = 0;
};

// 자식 Job 묶음 공통
struct ET_API FETJobGroup : public FETJob
{
public:
	// 추가한 Job 을 반환 (생성 후 값 설정이나 하위 묶음 구성에 사용)
	template <typename T, typename... ArgTypes>
	T& Add(ArgTypes&&... Args)
	{
		static_assert(TIsDerivedFrom<T, FETJob>::IsDerived, "T must derive from FETJob.");

		TUniquePtr<T> NewJob = MakeUnique<T>(Forward<ArgTypes>(Args)...);
		T& NewJobRef = *NewJob;
		JobArray.Add(MoveTemp(NewJob));

		return NewJobRef;
	}

protected:
	TArray<TUniquePtr<FETJob>> JobArray;
};

// 자식 Job 을 순서대로 실행 (앞 Job 이 끝나면 같은 프레임에 다음 Job 시작)
struct ET_API FETJobChain : public FETJobGroup
{
public:
	virtual void Start(UWorld* InWorld) override;
	virtual EETJobStatus Tick(const float InDeltaTime) override;
	virtual void End(const bool bInCancelled) override;

private:
	TWeakObjectPtr<UWorld> World;
	int32 CurrentIndex = INDEX_NONE;
};

// 자식 Job 을 동시에 실행 (모두 끝나면 완료)
struct ET_API FETJobParallel : public FETJobGroup
{
public:
	virtual void Start(UWorld* InWorld) override;
	virtual EETJobStatus Tick(const float InDeltaTime) override;
	virtual void End(const bool bInCancelled) override;

private:
	TArray<bool> FinishedArray;
};
