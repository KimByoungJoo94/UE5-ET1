#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "InputCoreTypes.h"
#include "ETCheckpointWidget.generated.h"

class UButton;

// 체크포인트 상호작용 시 HUD 를 가리는 전체 화면 창
UCLASS()
class ET_API UETCheckpointWidget : public UETUserWidget
{
	GENERATED_BODY()

public:
	UETCheckpointWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	UFUNCTION()
	void OnCloseButtonClicked();

	void CloseCheckpointWidget();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> CloseButton;

	UPROPERTY(EditDefaultsOnly)
	TArray<FKey> CloseKeyArray;
};
