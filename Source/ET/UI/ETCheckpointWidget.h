#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "InputCoreTypes.h"
#include "ETCheckpointWidget.generated.h"

class UTextBlock;
class UETTextButtonWidget;
class UETCheckpointTravelWidget;

// 체크포인트 상호작용 시 HUD 를 가리는 전체 화면 창 (휴식, 이동, 닫기)
UCLASS()
class ET_API UETCheckpointWidget : public UETUserWidget
{
	GENERATED_BODY()

public:
	UETCheckpointWidget(const FObjectInitializer& ObjectInitializer);

public:
	// 열기 전에 현재 체크포인트 지정 (정보 갱신, 이동 창은 닫힌 상태로 시작)
	void SetCheckpointRowName(const FName InCheckpointRowName);

protected:
	virtual void NativeOnInitialized() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	// 이동 창이 열려 있으면 이동 창만 닫음
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	void OnRestButtonClicked();
	void OnTravelButtonClicked();
	void OnCloseButtonClicked();

	void CloseCheckpointWidget();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CheckpointNameText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETTextButtonWidget> RestButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETTextButtonWidget> TravelButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETTextButtonWidget> CloseButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETCheckpointTravelWidget> TravelWidget;

	UPROPERTY(EditDefaultsOnly)
	TArray<FKey> CloseKeyArray;

private:
	FName CheckpointRowName;
};
