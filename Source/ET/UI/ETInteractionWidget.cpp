#include "UI/ETInteractionWidget.h"
#include "Components/TextBlock.h"
#include "Character/ETPlayer.h"

void UETInteractionWidget::UpdateKeyText()
{
	if (AETPlayer* Player = GetOwningETPlayer())
	{
		const FText& PrimaryActionKeyText = Player->GetPrimaryActionKeyText();		
		KeyText->SetText(PrimaryActionKeyText);
	}
}