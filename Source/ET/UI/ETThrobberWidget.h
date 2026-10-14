#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "ETThrobberWidget.generated.h"

class UWidgetAnimation;

enum class EETThrobberPosition : uint8
{
	TopLeft,
	TopRight,
	Center,
};

// 기다리는 동안 표시하는 공용 위젯 (UETGameInstance::ShowThrobber / HideThrobber 로 사용)
// Hide 가 빨리 와도 최소 시간은 표시, WBP 에 LoopAnimation 이름의 애니메이션이 있으면 표시 동안 반복 재생
UCLASS(Abstract)
class ET_API UETThrobberWidget : public UETUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

public:
	void Show();
	void Hide();

	// 뷰포트에 추가된 뒤 호출
	void SetThrobberPosition(const EETThrobberPosition InPosition);

protected:
	void FinishHide();

protected:	
	UPROPERTY(EditAnywhere, Category = "Throbber", meta = (ClampMin = "0.0"))
	float MinDisplayTime = 3.0f;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> LoopAnimation;

private:
	bool bShowing = false;
	bool bHideRequested = false;
	float DisplayElapsedTime = 0.f;
};
