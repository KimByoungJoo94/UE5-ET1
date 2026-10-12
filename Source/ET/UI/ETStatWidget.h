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

public:
	// 바인딩된 스탯 컴포넌트가 있으면 새 타입으로 다시 바인딩하고 갱신
	void SetGameStatType(const EETGameStatType InGameStatType);

	// 수치 텍스트 표시 여부 (false 면 Collapsed)
	void SetUseStatText(const bool bInUseStatText);

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

	UPROPERTY(EditAnywhere, Category = "Stat")
	bool bUseStatText = true;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ValueProgressBar;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ValueText;

private:
	void BindGameStatComponent();
	void UnbindGameStatComponent();

	void UpdateStatTextVisibility();

private:
	TWeakObjectPtr<UETGameStatComponent> BoundGameStatComponent;

	FDelegateHandle OnUpdateGameStatHandle;
};
