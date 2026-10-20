#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "DataAsset/ETGameStatTableRowData.h"
#include "ETStatWidget.generated.h"

class UProgressBar;
class UTextBlock;
struct FETGameStat;

UENUM()
enum class EETStatTextFormat : uint8
{	
	CurrentAndMax,
	Current,
	Max,
};

// 스탯 하나 표시 (WBP 에 있는 위젯만 갱신 : 막대, 스탯 이름, 수치)
UCLASS()
class ET_API UETStatWidget : public UETUserWidget
{
	GENERATED_BODY()

public:	
	void SetGameStatType(const EETGameStatType InGameStatType);
	void SetUseStatText(const bool bInUseStatText);
	void SetStatTextFormat(const EETStatTextFormat InStatTextFormat);
	void SetProgressBarColorName(const FName InProgressBarColorName);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnUpdateGameStat(const FETGameStat& InGameStat);
	void UpdateGameStat(const FETGameStat& InGameStat);

protected:
	UPROPERTY(EditAnywhere, Category = "Stat")
	EETGameStatType GameStatType = EETGameStatType::Health;

	// 엔진 GColorList 의 색 이름
	UPROPERTY(EditAnywhere, Category = "Stat", meta = (GetOptions = "ET.ETUserWidget.GetColorNameOptions"))
	FName ProgressBarColorName = TEXT("red");

	UPROPERTY(EditAnywhere, Category = "Stat")
	bool bUseStatText = true;

	UPROPERTY(EditAnywhere, Category = "Stat")
	EETStatTextFormat StatTextFormat = EETStatTextFormat::CurrentAndMax;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UProgressBar> ValueProgressBar;

	UPROPERTY(meta = (BindWidgetOptional))
	TObjectPtr<UTextBlock> DisplayNameText;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ValueText;

private:
	void BindGameStatComponent();
	void UnbindGameStatComponent();

	void UpdateStatTextVisibility();
	void UpdateProgressBarColor();
	// 스탯 이름은 우선 enum 표시 이름 사용
	void UpdateDisplayNameText();

private:
	TWeakObjectPtr<UETGameStatComponent> BoundGameStatComponent;

	FDelegateHandle OnUpdateGameStatHandle;
};
