#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "ETTextButtonWidget.generated.h"

class UButton;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE(FOnTextButtonClicked);

// 텍스트 버튼 (스타일은 WBP, 텍스트는 디자이너 또는 코드에서 지정)
UCLASS()
class ET_API UETTextButtonWidget : public UETUserWidget
{
	GENERATED_BODY()

public:
	void SetButtonText(const FText& InButtonText);

	// 비활성 모양은 WBP 버튼 스타일의 Disabled 로 표시
	void SetButtonEnabled(const bool bInEnabled);

	// WBP 버튼 스타일 브러시에 곱해지는 색 (기본 흰색)
	void SetButtonBackgroundColor(const FLinearColor& InButtonBackgroundColor);

protected:
	virtual void NativePreConstruct() override;
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	UFUNCTION()
	void OnButtonClicked();

public:
	FOnTextButtonClicked OnClicked;

protected:
	UPROPERTY(EditAnywhere, Category = "Button")
	FText ButtonText;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UButton> Button;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> ButtonTextBlock;
};
