#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "ETPlayerHUDWidget.generated.h"

class UETBossInfoWidget;
class AETCharacter;

UCLASS()
class ET_API UETPlayerHUDWidget : public UETUserWidget
{
	GENERATED_BODY()

public:
	void ShowBossInfo(AETCharacter* InBossCharacter);
	void HideBossInfo(const AETCharacter* InBossCharacter);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETBossInfoWidget> BossInfoWidget;
};
