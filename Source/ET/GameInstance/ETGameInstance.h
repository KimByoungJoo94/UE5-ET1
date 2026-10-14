// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "UI/ETThrobberWidget.h"
#include "ETGameInstance.generated.h"

// 레벨과 무관하게 유지되는 공용 UI (쓰로버) 관리 및 서브시스템 알림과 UI 연결
UCLASS()
class ET_API UETGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;
	virtual void Shutdown() override;

public:
	// 요청자가 하나라도 남아 있으면 표시 (같은 요청자의 중복 Show 는 한 번으로 취급)
	void ShowThrobber(const UObject* InRequester, const EETThrobberPosition InPosition = EETThrobberPosition::TopRight);
	void HideThrobber(const UObject* InRequester);

private:
	void RemoveInvalidThrobberRequesters();

	void OnSaveStarted();
	void OnSaveFinished();

private:
	UPROPERTY(Transient)
	TObjectPtr<UETThrobberWidget> ThrobberWidget;

	// Hide 없이 사라진 요청자는 다음 Show / Hide 때 정리
	TSet<TWeakObjectPtr<const UObject>> ThrobberRequesterSet;

	FDelegateHandle OnSaveStartedHandle;
	FDelegateHandle OnSaveFinishedHandle;
};
