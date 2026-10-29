#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "Components/SlateWrapperTypes.h"
#include "ETPlayerController.generated.h"

class UETPlayerHUDWidget;
class UETCheckpointWidget;
class UETMenuWidget;

UCLASS()
class ET_API AETPlayerController : public APlayerController
{
	GENERATED_BODY()
	
public:
	virtual void BeginPlay() override;

public:
	FORCEINLINE UETPlayerHUDWidget* GetPlayerHUDWidget() const { return PlayerHUDWidget; }

	// 열려 있는 동안 HUD 숨김, 입력은 UI 에서만 처리
	void OpenCheckpointWidget(const FName InCheckpointRowName);
	void CloseCheckpointWidget();

	// 체크포인트 휴식 (상태 회복, 몬스터 리스폰 / 부활 위치는 상호작용 시 이미 갱신됨)
	void RestAtCheckpoint();

	// 열려 있는 동안 HUD 숨김, 게임 일시정지, 입력은 UI 에서만 처리 (다른 창이 열려 있으면 무시)
	void OpenMenuWidget();
	void CloseMenuWidget();

protected:
	UPROPERTY(Transient)
	TObjectPtr<UETPlayerHUDWidget> PlayerHUDWidget;

	UPROPERTY(Transient)
	TObjectPtr<UETCheckpointWidget> CheckpointWidget;

	UPROPERTY(Transient)
	TObjectPtr<UETMenuWidget> MenuWidget;

private:
	ESlateVisibility PlayerHUDVisibility = ESlateVisibility::SelfHitTestInvisible;
};
