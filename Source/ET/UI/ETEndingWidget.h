#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "ETEndingWidget.generated.h"

class UETTextButtonWidget;
class UWidgetAnimation;

// 엔딩 (표시 시간이 지나거나 넘어가기 버튼을 누르면 타이틀 레벨로 / AETUIGameMode 가 생성)
// WBP 에 ShowAnimation 이름의 애니메이션이 있으면 시작 시 재생
UCLASS()
class ET_API UETEndingWidget : public UETUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnSkipButtonClicked();

private:
	// UI 입력 모드가 타이틀 레벨까지 남지 않도록 게임 입력으로 되돌린 뒤 타이틀 레벨 열기
	void ReturnToTitle();

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETTextButtonWidget> SkipButton;

	UPROPERTY(Transient, meta = (BindWidgetAnimOptional))
	TObjectPtr<UWidgetAnimation> ShowAnimation;

	// 시작 후 타이틀로 넘어가기까지 시간 (0 이하면 넘어가기 버튼으로만)
	UPROPERTY(EditAnywhere)
	float DisplayDuration = 10.f;

private:
	FTimerHandle ReturnToTitleTimerHandle;

	bool bReturningToTitle = false;
};
