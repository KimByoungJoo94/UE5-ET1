#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "ETInteractionWidget.generated.h"

class UTextBlock;

UCLASS()
class ET_API UETInteractionWidget : public UETUserWidget
{
	GENERATED_BODY()
	
public:
	void UpdateKeyText();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> KeyText;
};
