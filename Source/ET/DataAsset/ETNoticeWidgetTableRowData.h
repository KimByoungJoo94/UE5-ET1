#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETNoticeWidgetTableRowData.generated.h"

class UETNoticeWidget;

// 알림 위젯 (FETShowNoticeWidgetJob 에서 RowName 으로 표시)
USTRUCT()
struct ET_API FETNoticeWidgetTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UETNoticeWidget> WidgetClass;

	// 0 이하면 직접 닫기 전까지 유지
	UPROPERTY(EditAnywhere)
	float Duration = 3.f;

	UPROPERTY(EditAnywhere)
	int32 ZOrder = 10;
};
