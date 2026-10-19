#include "UI/ETTextButtonWidget.h"
#include "Components/Button.h"
#include "Components/TextBlock.h"

void UETTextButtonWidget::SetButtonText(const FText& InButtonText)
{
	ButtonText = InButtonText;

	if (ButtonTextBlock)
	{
		ButtonTextBlock->SetText(ButtonText);
	}
}

void UETTextButtonWidget::SetButtonEnabled(const bool bInEnabled)
{
	Button->SetIsEnabled(bInEnabled);
}

void UETTextButtonWidget::SetButtonBackgroundColor(const FLinearColor& InButtonBackgroundColor)
{
	Button->SetBackgroundColor(InButtonBackgroundColor);
}

void UETTextButtonWidget::NativePreConstruct()
{
	Super::NativePreConstruct();

	// 디자이너 미리보기에도 반영
	SetButtonText(ButtonText);
}

void UETTextButtonWidget::NativeConstruct()
{
	Super::NativeConstruct();

	Button->OnClicked.AddDynamic(this, &ThisClass::OnButtonClicked);
}

void UETTextButtonWidget::NativeDestruct()
{
	Button->OnClicked.RemoveDynamic(this, &ThisClass::OnButtonClicked);

	Super::NativeDestruct();
}

void UETTextButtonWidget::OnButtonClicked()
{
	OnClicked.Broadcast();
}
