#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "ETPlayerController.generated.h"

class UETPlayerHUDWidget;

UCLASS()
class ET_API AETPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UETPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY(Transient)
	TObjectPtr<UETPlayerHUDWidget> PlayerHUDWidget;
};
