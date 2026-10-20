#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "ETTitleWidget.generated.h"

class UETTextButtonWidget;

// 메인 타이틀 (이어하기, 새로하기, 종료 / AETUIGameMode 가 생성)
UCLASS()
class ET_API UETTitleWidget : public UETUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnContinueButtonClicked();
	void OnNewGameButtonClicked();
	void OnQuitButtonClicked();

private:
	// 세이브 파일이 있고 마지막 레벨이 레벨 테이블에 있으면 이어하기 가능
	bool CanContinue() const;

	// UI 입력 모드가 다음 레벨까지 남지 않도록 게임 입력으로 되돌린 뒤 레벨 열기
	void OpenLevel(const FName InLevelRowName);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETTextButtonWidget> ContinueButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETTextButtonWidget> NewGameButton;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETTextButtonWidget> QuitButton;
};
