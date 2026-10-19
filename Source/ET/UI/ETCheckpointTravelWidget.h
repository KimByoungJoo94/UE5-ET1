#pragma once

#include "CoreMinimal.h"
#include "UI/ETUserWidget.h"
#include "ETCheckpointTravelWidget.generated.h"

class UPanelWidget;
class UETTextButtonWidget;

// 체크포인트 창 안의 이동 창 (왼쪽 레벨 목록, 오른쪽 선택한 레벨의 체크포인트 목록)
// 테이블 전체를 보여주고 발견한 곳만 클릭 가능, 클릭하면 선택, 선택된 체크포인트를 한 번 더 클릭하면 이동
UCLASS()
class ET_API UETCheckpointTravelWidget : public UETUserWidget
{
	GENERATED_BODY()

public:
	// 현재 체크포인트와 그 레벨을 선택한 상태로 열기
	void OpenTravel(const FName InCurrentCheckpointRowName);
	void CloseTravel();
	bool IsTravelOpen() const;

protected:
	virtual void NativeConstruct() override;
	virtual void NativeDestruct() override;

	void OnBackButtonClicked();
	void OnLevelButtonClicked(FName InLevelRowName);
	void OnCheckpointButtonClicked(FName InCheckpointRowName);

private:
	// 레벨 테이블 순서, 발견한 체크포인트가 있는 레벨만 활성
	void RebuildLevelButtons();
	// 선택한 레벨의 체크포인트를 테이블 순서로, 발견한 곳만 활성
	void RebuildCheckpointButtons();
	// 선택된 버튼만 선택 배경색
	void UpdateButtonBackgroundColors();

	// 생성한 버튼의 클릭 바인딩 해제 후 패널, 맵 비우기
	void ClearButtons(UPanelWidget* InPanel, OUT TMap<FName, TObjectPtr<UETTextButtonWidget>>& OutButtonMap);

	UETTextButtonWidget* CreateButton(UPanelWidget* InPanel, const FText& InButtonText, const bool bInEnabled);

protected:
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> LevelButtonPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UPanelWidget> CheckpointButtonPanel;

	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UETTextButtonWidget> BackButton;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UETTextButtonWidget> ButtonClass;

	UPROPERTY(EditDefaultsOnly)
	FLinearColor SelectedBackgroundColor = FLinearColor(1.f, 0.7f, 0.15f);

	UPROPERTY(EditDefaultsOnly)
	FLinearColor NormalBackgroundColor = FLinearColor::White;

private:
	// RowName → 버튼 (선택 색 갱신용)
	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UETTextButtonWidget>> LevelButtonMap;

	UPROPERTY(Transient)
	TMap<FName, TObjectPtr<UETTextButtonWidget>> CheckpointButtonMap;

	FName CurrentCheckpointRowName;
	FName SelectedLevelRowName;
	FName SelectedCheckpointRowName;
};
