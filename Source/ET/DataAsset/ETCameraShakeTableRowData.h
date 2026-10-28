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

	// 0 이면 셰이크 에셋 기본 재생 시간, 0 보다 크면 이 시간(초)으로 덮어씀
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float Duration = 0.f;

	// 진원지가 있는 재생(범위 공격 등)에서만 사용, Inner 안은 Scale 그대로, Outer 까지 감쇠, Outer 밖은 재생 안 함
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float InnerRadius = 300.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float OuterRadius = 1500.f;
};
