#include "Subsystem/ETSaveSubsystem.h"
#include "Common/ETNames.h"
#include "Kismet/GameplayStatics.h"
#include "HAL/IConsoleManager.h"

void UETSaveSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);

	if (HasSaveGameInSlot())
	{
		SaveGame = Cast<UETSaveGame>(UGameplayStatics::LoadGameFromSlot(ETNames::SaveSlot::Default, ETNames::SaveSlot::UserIndex));
	}

	if (SaveGame == nullptr)
	{
		SaveGame = Cast<UETSaveGame>(UGameplayStatics::CreateSaveGameObject(UETSaveGame::StaticClass()));
		return;
	}

	// 이전 버전 데이터 변환은 버전이 올라갈 때 여기에 추가
	SaveGame->SaveVersion = UETSaveGame::LatestSaveVersion;
}

void UETSaveSubsystem::Deinitialize()
{
	if (SaveGame)
	{
		UGameplayStatics::SaveGameToSlot(SaveGame, ETNames::SaveSlot::Default, ETNames::SaveSlot::UserIndex);
	}

	Super::Deinitialize();
}

void UETSaveSubsystem::Save()
{
	if (bSaving)
	{
		bSavePending = true;
		return;
	}

	OnSaveStarted.Broadcast();

	StartAsyncSave();
}

void UETSaveSubsystem::StartAsyncSave()
{
	// 데이터는 호출 시점에 메모리로 직렬화되고 파일 쓰기만 비동기로 진행
	bSaving = true;
	UGameplayStatics::AsyncSaveGameToSlot(SaveGame, ETNames::SaveSlot::Default, ETNames::SaveSlot::UserIndex,
		FAsyncSaveGameToSlotDelegate::CreateUObject(this, &ThisClass::OnAsyncSaveFinished));
}

bool UETSaveSubsystem::HasSaveGameInSlot() const
{
	return UGameplayStatics::DoesSaveGameExist(ETNames::SaveSlot::Default, ETNames::SaveSlot::UserIndex);
}

void UETSaveSubsystem::ResetSaveGame()
{
	SaveGame = Cast<UETSaveGame>(UGameplayStatics::CreateSaveGameObject(UETSaveGame::StaticClass()));

	// 저장 중이면 쓰기가 끝난 뒤 삭제 (먼저 지우면 진행 중인 저장이 이전 데이터로 다시 만듦)
	if (bSaving)
	{
		bDeletePending = true;
		bSavePending = false;
		return;
	}

	UGameplayStatics::DeleteGameInSlot(ETNames::SaveSlot::Default, ETNames::SaveSlot::UserIndex);
}

void UETSaveSubsystem::OnAsyncSaveFinished(const FString& InSlotName, const int32 InUserIndex, bool bInSuccess)
{
	bSaving = false;

	ensureMsgf(bInSuccess, TEXT("Failed to save game. Slot: %s"), *InSlotName);

	if (bDeletePending)
	{
		bDeletePending = false;
		UGameplayStatics::DeleteGameInSlot(ETNames::SaveSlot::Default, ETNames::SaveSlot::UserIndex);
	}

	// 대기 중인 저장이 있으면 이어서 저장하고, 모두 끝났을 때만 완료 알림
	if (bSavePending)
	{
		bSavePending = false;
		StartAsyncSave();
		return;
	}

	OnSaveFinished.Broadcast();
}

#if !UE_BUILD_SHIPPING
// 콘솔 : ET.ResetSaveGame (PIE 에서도 세이브 파일이 남으므로 처음부터 테스트할 때 사용)
static FAutoConsoleCommandWithWorld ResetSaveGameCommand(
	TEXT("ET.ResetSaveGame"),
	TEXT("Reset save data and delete save slot file."),
	FConsoleCommandWithWorldDelegate::CreateLambda([](UWorld* InWorld)
	{
		if (UETSaveSubsystem* SaveSubsystem = ETSubsystem::GetPtr<UETSaveSubsystem>(InWorld))
		{
			SaveSubsystem->ResetSaveGame();
		}
	}));
#endif
