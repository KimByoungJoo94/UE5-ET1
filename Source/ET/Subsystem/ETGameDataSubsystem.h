#pragma once

#include "CoreMinimal.h"
#include "Subsystems/GameInstanceSubsystem.h"
#include "ETGameDataSubsystem.generated.h"

struct FETGameStatTableRowBase;

UCLASS()
class ET_API UETGameDataSubsystem : public UGameInstanceSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

public:
	const FETGameStatTableRowBase* GetGameStatRow(const FName InRowName) const;

private:
	UPROPERTY()
	TObjectPtr<UDataTable> BaseGameStatDataTable;
};
