#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "InputCoreTypes.h"
#include "ETMenuWidget.generated.h"

class UETTextButtonWidget;
class UETStatListWidget;
class UETStatWidget;

// 플레이 중 메뉴 (열려 있는 동안 게임 일시정지, 플레이어 스탯 표시, 게임 복귀 / 타이틀 / 게임 종료)
UCLASS()
class ET_API UETMenuWidget : public UETUserWidget
{
	GENERATED_BODY()

public:
	UETMenuWidget(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void NativeOnInitialized() override;
	// 열 때마다 소유 캐릭터 기준으로 스탯 목록 다시 생성
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;
	virtual FReply NativeOnKeyDown(const FGeometry& InGeometry, const FKeyEvent& InKeyEvent) override;

	void OnReturnToGameButtonClicked();
	void OnReturnToCheckpointButtonClicked();
	void OnReturnToTitleButtonClicked();
	void OnQuitGameButtonClicked();

	void CloseMenuWidget();

private:
	// 소유 캐릭터가 가진 스탯 전부를 enum 순서로 표시 (최대값 0 제외)
	void BuildStatList();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETTextButtonWidget> ReturnToGameButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETTextButtonWidget> ReturnToTitleButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETTextButtonWidget> ReturnToCheckpointButton;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETTextButtonWidget> QuitGameButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETStatListWidget> StatListWidget;

	// 스탯 목록 항목 WBP (스탯 이름, 수치)
	UPROPERTY(EditDefaultsOnly)
	TSoftClassPtr<UETStatWidget> StatWidgetClass;

	UPROPERTY(EditDefaultsOnly)
	TArray<FKey> CloseKeyArray;
};
