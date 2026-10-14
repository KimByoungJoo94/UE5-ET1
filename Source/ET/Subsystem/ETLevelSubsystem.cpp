#include "Subsystem/ETLevelSubsystem.h"
#include "Kismet/GameplayStatics.h"

void UETLevelSubsystem::RestartLevel()
{
	UGameplayStatics::OpenLevel(this, FName(UGameplayStatics::GetCurrentLevelName(this)));
}
