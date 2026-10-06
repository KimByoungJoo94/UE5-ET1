#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ETUserWidget.generated.h"

class AETCharacter;
class AETPlayer;
class UETGameStatComponent;

UCLASS()
class ET_API UETUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	AETCharacter* GetOwningETCharacter() const;
	AETPlayer* GetOwningETPlayer() const;
	UETGameStatComponent* GetOwningGameStatComponent() const;
};
