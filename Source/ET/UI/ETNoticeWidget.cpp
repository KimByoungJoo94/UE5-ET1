#include "UI/ETNoticeWidget.h"
#include "Animation/WidgetAnimation.h"

void UETNoticeWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (RemainingTime <= 0.f || bClosing)
	{
		return;
	}

	RemainingTime -= InDeltaTime;
	if (RemainingTime <= 0.f)
	{
		Close();
	}
}

void UETNoticeWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	if (bClosing && Animation == HideAnimation)
	{
		FinishClose();
	}
}

void UETNoticeWidget::Show(const float InDuration)
{
	RemainingTime = InDuration;
	bClosing = false;
	bClosed = false;

	if (ShowAnimation)
	{
		PlayAnimation(ShowAnimation);
	}
}

void UETNoticeWidget::Close()
{
	if (bClosing || bClosed)
	{
		return;
	}

	bClosing = true;

	if (HideAnimation)
	{
		PlayAnimation(HideAnimation);
		return;
	}

	FinishClose();
}

void UETNoticeWidget::FinishClose()
{
	RemoveFromParent();

	bClosing = false;
	bClosed = true;
}
