#include "Job/ETWidgetJobs.h"
#include "UI/ETNoticeWidget.h"
#include "DataAsset/ETNoticeWidgetTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "GameFramework/PlayerController.h"

FETShowNoticeWidgetJob::FETShowNoticeWidgetJob(APlayerController* InPlayerController, const FName InNoticeWidgetRowName)
	: PlayerController(InPlayerController)
	, NoticeWidgetRowName(InNoticeWidgetRowName)
{
}

void FETShowNoticeWidgetJob::Start(UWorld* InWorld)
{
	const FETNoticeWidgetTableRowData* RowData = ETSubsystem::Get<UETGameDataSubsystem>(InWorld).GetRow<FETNoticeWidgetTableRowData>(NoticeWidgetRowName);
	UClass* WidgetClass = RowData ? RowData->WidgetClass.LoadSynchronous() : nullptr;
	if (PlayerController.IsValid() == false || WidgetClass == nullptr)
	{
		return;
	}

	NoticeWidget = CreateWidget<UETNoticeWidget>(PlayerController.Get(), WidgetClass);
	if (NoticeWidget.IsValid())
	{
		NoticeWidget->AddToViewport(RowData->ZOrder);
		NoticeWidget->Show(RowData->Duration);
	}
}

EETJobStatus FETShowNoticeWidgetJob::Tick(const float InDeltaTime)
{
	// 다른 곳에서 뷰포트에서 제거한 경우도 완료
	if (NoticeWidget.IsValid() == false || NoticeWidget->IsClosed() || NoticeWidget->IsInViewport() == false)
	{
		return EETJobStatus::Finished;
	}

	return EETJobStatus::Running;
}

void FETShowNoticeWidgetJob::End(const bool bInCancelled)
{
	// 취소되면 퇴장 애니메이션 없이 바로 제거
	if (bInCancelled && NoticeWidget.IsValid())
	{
		NoticeWidget->RemoveFromParent();
	}

	NoticeWidget.Reset();
}
