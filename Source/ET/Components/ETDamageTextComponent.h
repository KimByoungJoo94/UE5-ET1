#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ETDamageTextComponent.generated.h"

class UWidgetComponent;

// 입힌 데미지 숫자를 맞은 위치에 표시 (위젯 컴포넌트를 미리 만들어 두고 오래된 순으로 재사용)
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETDamageTextComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	void ShowDamageText(const FVector& InLocation, const float InDamage);

protected:
	virtual void BeginPlay() override;

protected:
	// 동시에 표시 가능한 최대 개수
	UPROPERTY(EditAnywhere, Category = "DamageText", meta = (ClampMin = "1"))
	int32 PoolSize = 8;

	// 위젯 크기 기준 피벗 (0.5, 0.5 = 맞은 위치 중앙, Y 가 클수록 위로)
	UPROPERTY(EditAnywhere, Category = "DamageText")
	FVector2D TextPivot = FVector2D(0.5f, 2.f);

	// 피벗 X 랜덤 범위 (± 위젯 너비 비율)
	UPROPERTY(EditAnywhere, Category = "DamageText", meta = (ClampMin = "0.0"))
	float RandomPivotRangeX = 0.5f;

private:
	UPROPERTY(Transient)
	TArray<TObjectPtr<UWidgetComponent>> WidgetComponentArray;

	int32 NextWidgetComponentIndex = 0;
};
