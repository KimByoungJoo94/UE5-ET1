#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "ETPlayerHUDWidget.generated.h"

class UProgressBar;
struct FETGameStat;

UCLASS()
class ET_API UETPlayerHUDWidget : public UETUserWidget
{
	GENERATED_BODY()
	
protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	
public:
	void UpdateHealth();

	void OnUpdateGameStat(const FETGameStat& InGameStat);

protected:
	UPROPERTY()
	TObjectPtr<UProgressBar> HealthProgressBar;

private:
	FDelegateHandle OnUpdateGameStatHandle;
};
