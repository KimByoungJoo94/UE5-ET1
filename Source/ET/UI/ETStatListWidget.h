#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "DataAsset/ETGameStatTableRowData.h"
#include "ETStatListWidget.generated.h"

class UETStatWidget;
class UVerticalBox;

USTRUCT()
struct FETStatBarData
{
	GENERATED_BODY()

public:
	FETStatBarData() = default;
	FETStatBarData(const EETGameStatType InGameStatType, const FName InProgressBarColorName)
		: GameStatType(InGameStatType), ProgressBarColorName(InProgressBarColorName)
	{
	}

	UPROPERTY(EditAnywhere)
	EETGameStatType GameStatType = EETGameStatType::Health;

	// 엔진 GColorList 의 색 이름
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETStatWidget.GetColorNameOptions"))
	FName ProgressBarColorName = TEXT("red");
};

USTRUCT()
struct FETStatListSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UETStatWidget> StatWidgetClass;

	// 위에서부터 순서대로 표시 (최대값이 0 인 스탯은 제외)
	UPROPERTY(EditAnywhere)
	TArray<FETStatBarData> StatBarDataArray;

	UPROPERTY(EditAnywhere)
	FVector2D StatBarSize = FVector2D(130.f, 10.f);

	UPROPERTY(EditAnywhere)
	float StatBarSpacing = 2.f;

	UPROPERTY(EditAnywhere)
	bool bUseStatText = false;
};

// WBP 없이 코드로 스탯 위젯을 세로로 나열
UCLASS()
class ET_API UETStatListWidget : public UETUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	// 기존 스탯 위젯을 지우고 소유 캐릭터 기준으로 다시 생성, 전체 크기 반환
	FVector2D BuildStatWidgets(const FETStatListSettings& InStatListSettings);

private:
	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> StatVerticalBox;
};
