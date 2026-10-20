#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "DataAsset/ETGameStatTableRowData.h"
#include "UI/ETStatWidget.h"
#include "ETStatListWidget.generated.h"

class UVerticalBox;

// 스탯 위젯 하나의 표시 설정 (막대, 정보 등 WBP 공용)
USTRUCT()
struct FETStatUISettingData
{
	GENERATED_BODY()

public:
	FETStatUISettingData() = default;
	FETStatUISettingData(const EETGameStatType InGameStatType, const FName InProgressBarColorName)
		: GameStatType(InGameStatType), ProgressBarColorName(InProgressBarColorName)
	{
	}

	UPROPERTY(EditAnywhere)
	EETGameStatType GameStatType = EETGameStatType::Health;

	// 엔진 GColorList 의 색 이름 (막대가 없는 WBP 는 무시)
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETUserWidget.GetColorNameOptions"))
	FName ProgressBarColorName = TEXT("red");
};

USTRUCT()
struct FETStatListSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UETStatWidget> StatWidgetClass;

	// 위에서부터 순서대로 표시
	UPROPERTY(EditAnywhere)
	TArray<FETStatUISettingData> StatUISettingDataArray;

	// 항목 크기 고정 (X, Y 중 하나라도 0 이하면 WBP 크기 그대로)
	UPROPERTY(EditAnywhere)
	FVector2D StatWidgetSize = FVector2D(130.f, 10.f);

	UPROPERTY(EditAnywhere)
	float StatWidgetSpacing = 2.f;

	UPROPERTY(EditAnywhere)
	bool bUseStatText = false;

	UPROPERTY(EditAnywhere)
	EETStatTextFormat StatTextFormat = EETStatTextFormat::CurrentAndMax;

	// 최대값이 0 인 스탯 제외 (몬스터마다 없는 스탯 숨김)
	UPROPERTY(EditAnywhere)
	bool bHideZeroMaxStat = true;
};

// WBP 없이 코드로 스탯 위젯을 세로로 나열 (다른 WBP 에 배치해서 사용 가능)
UCLASS()
class ET_API UETStatListWidget : public UETUserWidget
{
	GENERATED_BODY()

public:
	virtual bool Initialize() override;

	// 기존 스탯 위젯을 지우고 소유 캐릭터 기준으로 다시 생성, 전체 크기 반환 (항목 크기를 고정하지 않으면 0)
	FVector2D BuildStatWidgets(const FETStatListSettings& InStatListSettings);

private:
	UPROPERTY(Transient)
	TObjectPtr<UVerticalBox> StatVerticalBox;
};
