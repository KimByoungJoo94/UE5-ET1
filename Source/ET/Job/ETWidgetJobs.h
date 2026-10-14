#pragma once

#include "CoreMinimal.h"
#include "Job/ETJob.h"

class APlayerController;
class UETNoticeWidget;

// 알림 위젯 테이블의 위젯을 띄우고 위젯이 닫히면 완료
struct ET_API FETShowNoticeWidgetJob : public FETJob
{
public:
	FETShowNoticeWidgetJob(APlayerController* InPlayerController, const FName InNoticeWidgetRowName);

	virtual void Start(UWorld* InWorld) override;
	virtual EETJobStatus Tick(const float InDeltaTime) override;
	virtual void End(const bool bInCancelled) override;

private:
	TWeakObjectPtr<APlayerController> PlayerController;
	FName NoticeWidgetRowName;

	TWeakObjectPtr<UETNoticeWidget> NoticeWidget;
};
