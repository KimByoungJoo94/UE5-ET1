#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "ETBossInfoWidget.generated.h"

class UETStatWidget;
class UTextBlock;

// 플레이어 HUD 의 보스 이름, 체력, 스턴 수치 (보스를 지정하면 표시, 해제하면 숨김)
UCLASS()
class ET_API UETBossInfoWidget : public UETUserWidget
{
	GENERATED_BODY()

public:
	void SetBossCharacter(AETCharacter* InBossCharacter);
	AETCharacter* GetBossCharacter() const;

protected:
	virtual void NativeConstruct() override;

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETStatWidget> HealthBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETStatWidget> StunPointBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> NameText;

private:
	TWeakObjectPtr<AETCharacter> BossCharacter;
};
