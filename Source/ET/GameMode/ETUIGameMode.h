#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ETUIGameMode.generated.h"

class UUserWidget;

// 플레이 없이 위젯만 보여주는 레벨 (타이틀, 엔딩 / 폰 없이 위젯 생성 후 UI 입력)
UCLASS()
class ET_API AETUIGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AETUIGameMode();

protected:
	virtual void BeginPlay() override;

protected:
	UPROPERTY(EditAnywhere)
	TSubclassOf<UUserWidget> WidgetClass;
};
