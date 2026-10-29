#include "UI/ETBossInfoWidget.h"
#include "UI/ETStatWidget.h"
#include "Character/ETCharacter.h"
#include "Components/ETGameStatComponent.h"
#include "Components/TextBlock.h"
#include "DataAsset/ETCharacterTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"

void UETBossInfoWidget::NativeConstruct()
{
	Super::NativeConstruct();

	// 보스를 지정하기 전에는 숨김
	SetVisibility(ESlateVisibility::Collapsed);
}

void UETBossInfoWidget::SetBossCharacter(AETCharacter* InBossCharacter)
{
	if (BossCharacter.Get() == InBossCharacter)
	{
		return;
	}

	BossCharacter = InBossCharacter;

	if (InBossCharacter == nullptr)
	{
		SetVisibility(ESlateVisibility::Collapsed);
		return;
	}

	SetOwningETCharacter(InBossCharacter);

	// 소유 캐릭터가 바뀌었으므로 스탯 위젯도 보스 기준으로 다시 바인딩
	HealthBar->SetOwningETCharacter(InBossCharacter);
	HealthBar->SetGameStatType(EETGameStatType::Health);

	// 스턴 면역(최대값 0)이면 숨김
	const bool bUseStunPoint = InBossCharacter->GetGameStatComponent()->GetGameStat(EETGameStatType::StunPoint).GetMaxValue() > 0.f;
	StunPointBar->SetOwningETCharacter(InBossCharacter);
	StunPointBar->SetGameStatType(EETGameStatType::StunPoint);
	StunPointBar->SetVisibility(bUseStunPoint ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);

	const FETCharacterTableRowData* CharacterRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETCharacterTableRowData>(InBossCharacter->GetCharacterRowName());
	NameText->SetText(CharacterRowData ? CharacterRowData->DisplayText : FText::GetEmpty());

	SetVisibility(ESlateVisibility::SelfHitTestInvisible);
}

AETCharacter* UETBossInfoWidget::GetBossCharacter() const
{
	return BossCharacter.Get();
}
