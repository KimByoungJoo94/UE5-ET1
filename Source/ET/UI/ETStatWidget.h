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
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void OnUpdateGameStat(const FETGameStat& InGameStat);
	void UpdateGameStat(const FETGameStat& InGameStat);

protected:
	UPROPERTY(EditAnywhere, Category = "Stat")
	EETGameStatType GameStatType = EETGameStatType::Health;

	// 엔진 GColorList 의 색 이름
	UPROPERTY(EditAnywhere, Category = "Stat", meta = (GetOptions = "ET.ETUserWidget.GetColorNameOptions"))
	FName ProgressBarColorName = TEXT("red");

	// 막대가 목표값을 따라가는 속도 (0 이면 즉시 반영)
	UPROPERTY(EditAnywhere, Category = "Stat", meta = (ClampMin = "0.0"))
	float ProgressBarInterpSpeed = 10.f;

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

	// 바인딩 직후 등 처음 표시할 때는 보간 없이 바로 반영
	void SetProgressBarPercent(const float InPercent, const bool bInImmediately);

private:
	TWeakObjectPtr<UETGameStatComponent> BoundGameStatComponent;

	FDelegateHandle OnUpdateGameStatHandle;

	// 막대 보간 (Tick 에서 표시값을 목표값으로)
	float TargetProgressBarPercent = 0.f;
	float DisplayProgressBarPercent = 0.f;
};
