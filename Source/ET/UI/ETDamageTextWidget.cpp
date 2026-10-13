#include "UI/ETDamageTextWidget.h"
#include "Components/TextBlock.h"
#include "Animation/WidgetAnimation.h"

void UETDamageTextWidget::ShowDamage(const float InDamage)
{
	DamageText->SetText(FText::AsNumber(FMath::RoundToInt32(InDamage)));
	SetVisibility(ESlateVisibility::HitTestInvisible);

	// 재생 중이어도 처음부터 다시 재생
	PlayAnimation(ShowAnimation);
}

void UETDamageTextWidget::NativeOnInitialized()
{
	Super::NativeOnInitialized();

	SetVisibility(ESlateVisibility::Collapsed);
}

void UETDamageTextWidget::OnAnimationFinished_Implementation(const UWidgetAnimation* Animation)
{
	Super::OnAnimationFinished_Implementation(Animation);

	if (Animation == ShowAnimation)
	{
		SetVisibility(ESlateVisibility::Collapsed);
	}
}
