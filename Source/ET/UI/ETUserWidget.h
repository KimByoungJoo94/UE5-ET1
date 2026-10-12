#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "ETUserWidget.generated.h"

class AETCharacter;
class AETPlayer;
class UETGameStatComponent;
class UETChargeAttackComponent;

UCLASS()
class ET_API UETUserWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	void SetOwningETCharacter(AETCharacter* InCharacter);

	AETCharacter* GetOwningETCharacter() const;
	AETPlayer* GetOwningETPlayer() const;
	UETGameStatComponent* GetOwningGameStatComponent() const;
	UETChargeAttackComponent* GetOwningChargeAttackComponent() const;

private:
	TWeakObjectPtr<AETCharacter> OwningETCharacter;
};
