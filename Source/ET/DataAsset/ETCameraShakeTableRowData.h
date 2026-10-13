#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETCameraShakeTableRowData.generated.h"

class UCameraShakeBase;

// 플레이어 카메라 흔들림 (같은 셰이크 에셋을 Scale 만 다르게 여러 행으로 사용 가능)
USTRUCT()
struct ET_API FETCameraShakeTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UCameraShakeBase> CameraShakeClass;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float Scale = 1.f;
};
