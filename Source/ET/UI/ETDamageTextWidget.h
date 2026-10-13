#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "ETDamageTextWidget.generated.h"

class UTextBlock;
class UWidgetAnimation;

// 데미지 숫자 (ShowAnimation 재생이 끝나면 숨김)
UCLASS()
class ET_API UETDamageTextWidget : public UETUserWidget
{
	GENERATED_BODY()

public:
	void ShowDamage(const float InDamage);

protected:
	virtual void NativeOnInitialized() override;
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> DamageText;

	UPROPERTY(Transient, meta = (BindWidgetAnim))
	TObjectPtr<UWidgetAnimation> ShowAnimation;
};
