#include "Job/ETLevelJobs.h"
#include "Subsystem/ETLevelSubsystem.h"

void FETRestartLevelJob::Start(UWorld* InWorld)
{
	// 레벨 이동은 다음 엔진 Tick 에 처리되므로 바로 완료
	ETSubsystem::Get<UETLevelSubsystem>(InWorld).RestartLevel();
}

EETJobStatus FETRestartLevelJob::Tick(const float InDeltaTime)
{
	return EETJobStatus::Finished;
}

void FETRestartLevelJob::End(const bool bInCancelled)
{
}
