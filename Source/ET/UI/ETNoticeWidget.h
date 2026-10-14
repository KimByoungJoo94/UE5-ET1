#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "ETNoticeWidget.generated.h"

class UWidgetAnimation;

// 표시 후 일정 시간이 지나면 스스로 닫히는 알림 위젯 (실제 시간 기준)
// WBP 에 ShowAnimation / HideAnimation 이름의 애니메이션이 있으면 등장 / 퇴장 시 재생
UCLASS(Abstract)
class ET_API UETNoticeWidget : public UETUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;
	virtual void OnAnimationFinished_Implementation(const UWidgetAnimation* Animation) override;

public:
	// InDuration 이 0 이하면 Close 호출 전까지 유지
	void Show(const float InDuration);
	void Close();

	FORCEINLINE bool IsClosed() const { return bClosed; }

protected:
	void FinishClose();

protected:
	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> ShowAnimation;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> HideAnimation;

private:
	float RemainingTime = 0.f;

	bool bClosing = false;
	bool bClosed = false;
};
