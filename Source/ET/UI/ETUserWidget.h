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

	// 에디터 드롭다운(GetOptions) 용 엔진 GColorList 의 색 이름 목록
	UFUNCTION()
	static TArray<FString> GetColorNameOptions();

private:
	TWeakObjectPtr<AETCharacter> OwningETCharacter;
};
