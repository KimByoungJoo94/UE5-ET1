#include "Subsystem/ETTimeDilationSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UETTimeDilationSubsystem::Deinitialize()
{
	StopTimeDilation();

	Super::Deinitialize();
}

void UETTimeDilationSubsystem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (GetWorld()->GetRealTimeSeconds() >= EndRealTimeSeconds)
	{
		StopTimeDilation();
	}
}

TStatId UETTimeDilationSubsystem::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(UETTimeDilationSubsystem, STATGROUP_Tickables);
}

void UETTimeDilationSubsystem::StartTimeDilation(const float InTimeDilation, const float InRealDuration)
{
	UGameplayStatics::SetGlobalTimeDilation(this, InTimeDilation);

	bTimeDilationActive = true;
	EndRealTimeSeconds = GetWorld()->GetRealTimeSeconds() + InRealDuration;
}

void UETTimeDilationSubsystem::StopTimeDilation()
{
	if (bTimeDilationActive == false)
	{
		return;
	}

	bTimeDilationActive = false;
	UGameplayStatics::SetGlobalTimeDilation(this, 1.f);
}
