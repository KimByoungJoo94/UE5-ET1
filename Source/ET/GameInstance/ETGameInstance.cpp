// Fill out your copyright notice in the Description page of Project Settings.


#include "GameInstance/ETGameInstance.h"
#include "Subsystem/ETSaveSubsystem.h"
#include "Common/ETCommonSettings.h"

void UETGameInstance::Init()
{
	// 서브시스템은 Super::Init 에서 생성
	Super::Init();

	UETSaveSubsystem* SaveSubsystem = GetSubsystem<UETSaveSubsystem>();
	OnSaveStartedHandle = SaveSubsystem->OnSaveStarted.AddUObject(this, &ThisClass::OnSaveStarted);
	OnSaveFinishedHandle = SaveSubsystem->OnSaveFinished.AddUObject(this, &ThisClass::OnSaveFinished);
}

void UETGameInstance::Shutdown()
{
	if (UETSaveSubsystem* SaveSubsystem = GetSubsystem<UETSaveSubsystem>())
	{
		SaveSubsystem->OnSaveStarted.Remove(OnSaveStartedHandle);
		SaveSubsystem->OnSaveFinished.Remove(OnSaveFinishedHandle);
	}

	Super::Shutdown();
}

void UETGameInstance::ShowThrobber(const UObject* InRequester, const EETThrobberPosition InPosition)
{
	RemoveInvalidThrobberRequesters();
	ThrobberRequesterSet.Add(InRequester);

	if (ThrobberWidget == nullptr)
	{
		if (UClass* ThrobberWidgetClass = GetDefault<UETCommonSettings>()->UI.ThrobberWidgetClass.LoadSynchronous())
		{
			ThrobberWidget = CreateWidget<UETThrobberWidget>(this, ThrobberWidgetClass);
		}
	}

	if (ThrobberWidget == nullptr)
	{
		return;
	}

	// 레벨 이동 시 뷰포트에서 제거되므로 다시 추가
	if (ThrobberWidget->IsInViewport() == false)
	{
		ThrobberWidget->AddToViewport(100);
	}

	ThrobberWidget->SetThrobberPosition(InPosition);
	ThrobberWidget->Show();
}

void UETGameInstance::HideThrobber(const UObject* InRequester)
{
	ThrobberRequesterSet.Remove(InRequester);
	RemoveInvalidThrobberRequesters();

	if (ThrobberRequesterSet.IsEmpty() && ThrobberWidget)
	{
		ThrobberWidget->Hide();
	}
}

void UETGameInstance::RemoveInvalidThrobberRequesters()
{
	for (auto It = ThrobberRequesterSet.CreateIterator(); It; ++It)
	{
		if (It->IsValid() == false)
		{
			It.RemoveCurrent();
		}
	}
}

void UETGameInstance::OnSaveStarted()
{
	ShowThrobber(GetSubsystem<UETSaveSubsystem>());
}

void UETGameInstance::OnSaveFinished()
{
	HideThrobber(GetSubsystem<UETSaveSubsystem>());
}
