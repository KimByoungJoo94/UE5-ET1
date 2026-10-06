#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "ETChargeAttackWidget.generated.h"

class UProgressBar;
class UTextBlock;

UCLASS()
class ET_API UETChargeAttackWidget : public UETUserWidget
{
	GENERATED_BODY()

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void InitWidget();
	void OnChargeCountChanged(const int32 InCurrentChargeCount, const int32 InMaxChargeCount);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UProgressBar> ProgressBar;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UTextBlock> CountText;

private:
	FDelegateHandle OnChargeCountChangedHandle;
};
