#include "UI/ETPlayerHUDWidget.h"
#include "UI/ETBossInfoWidget.h"

void UETPlayerHUDWidget::ShowBossInfo(AETCharacter* InBossCharacter)
{
	BossInfoWidget->SetBossCharacter(InBossCharacter);
}

void UETPlayerHUDWidget::HideBossInfo(const AETCharacter* InBossCharacter)
{
	if (BossInfoWidget->GetBossCharacter() == InBossCharacter)
	{
		BossInfoWidget->SetBossCharacter(nullptr);
	}
}
