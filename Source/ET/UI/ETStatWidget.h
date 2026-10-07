#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "DataAsset/ETGameStatTableRowData.h"
#include "ETStatWidget.generated.h"

class UProgressBar;
class UTextBlock;
struct FETGameStat;

UCLASS()
class ET_API UETStatWidget : public UETUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnUpdateGameStat(const FETGameStat& InGameStat);
	void UpdateGameStat(const FETGameStat& InGameStat);

	UFUNCTION()
	TArray<FString> GetColorNameOptions() const;

protected:
	UPROPERTY(EditAnywhere, Category = "Stat")
	EETGameStatType GameStatType = EETGameStatType::Health;

	// 엔진 GColorList 의 색 이름
	UPROPERTY(EditAnywhere, Category = "Stat", meta = (GetOptions = "GetColorNameOptions"))
	FName ProgressBarColorName = TEXT("red");

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ValueProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ValueText;

private:
	TWeakObjectPtr<UETGameStatComponent> BoundGameStatComponent;
	FDelegateHandle OnUpdateGameStatHandle;
};
