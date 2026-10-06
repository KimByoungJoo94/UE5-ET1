#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "ETTimeDilationSubsystem.generated.h"

UCLASS()
class ET_API UETTimeDilationSubsystem : public UTickableWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual void Deinitialize() override;
	virtual void Tick(float DeltaTime) override;
	virtual TStatId GetStatId() const override;
	virtual bool IsTickable() const override { return bTimeDilationActive; }

public:
	void StartTimeDilation(const float InTimeDilation, const float InRealDuration);
	void StopTimeDilation();

private:
	bool bTimeDilationActive = false;
	double EndRealTimeSeconds = 0.0;
};
