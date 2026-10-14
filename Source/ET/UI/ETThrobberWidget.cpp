#include "UI/ETThrobberWidget.h"
#include "Animation/WidgetAnimation.h"
#include "Blueprint/GameViewportSubsystem.h"

void UETThrobberWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (bShowing == false)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UETThrobberWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (bShowing == false)
	{
		return;
	}

	DisplayElapsedTime += InDeltaTime;

	if (bHideRequested && DisplayElapsedTime >= MinDisplayTime)
	{
		FinishHide();
	}
}

void UETThrobberWidget::Show()
{
	bHideRequested = false;

	if (bShowing)
	{
		return;
	}

	bShowing = true;
	DisplayElapsedTime = 0.f;

	SetVisibility(ESlateVisibility::HitTestInvisible);

	if (LoopAnimation)
	{
		PlayAnimation(LoopAnimation, 0.f, 0);
	}
}

void UETThrobberWidget::Hide()
{
	// 최소 표시 시간이 지나면 Tick 에서 숨김
	if (bShowing)
	{
		bHideRequested = true;
	}
}

void UETThrobberWidget::SetThrobberPosition(const EETThrobberPosition InPosition)
{
	// 앵커와 정렬을 같은 지점으로 맞추고 가장자리에서 40 만큼 안쪽으로
	FVector2D Point = FVector2D(0.5f, 0.5f);
	FVector2D Offset = FVector2D::ZeroVector;

	switch (InPosition)
	{
	case EETThrobberPosition::TopLeft:
		Point = FVector2D(0.f, 0.f);
		Offset = FVector2D(40.f, 40.f);
		break;
	case EETThrobberPosition::TopRight:
		Point = FVector2D(1.f, 0.f);
		Offset = FVector2D(-40.f, 40.f);
		break;
	case EETThrobberPosition::Center:
		break;
	}

	// SetPositionInViewport / SetDesiredSizeInViewport 는 앵커를 (0, 0) 으로 되돌리므로 슬롯을 한 번에 설정
	UGameViewportSubsystem* GameViewportSubsystem = UGameViewportSubsystem::Get();
	if (GameViewportSubsystem == nullptr)
	{
		return;
	}

	FGameViewportWidgetSlot ViewportSlot = GameViewportSubsystem->GetWidgetSlot(this);
	ViewportSlot.Anchors = FAnchors(Point.X, Point.Y);
	ViewportSlot.Alignment = Point;
	// 크기를 0 으로 두면 WBP 루트의 DesiredSize 사용 (루트가 Canvas Panel 이면 0 이 되어 보이지 않음)
	ViewportSlot.Offsets = FMargin(Offset.X, Offset.Y, 0.f, 0.f);

	GameViewportSubsystem->SetWidgetSlot(this, ViewportSlot);
}

void UETThrobberWidget::FinishHide()
{
	bShowing = false;
	bHideRequested = false;

	SetVisibility(ESlateVisibility::Collapsed);

	if (LoopAnimation)
	{
		StopAnimation(LoopAnimation);
	}
}
