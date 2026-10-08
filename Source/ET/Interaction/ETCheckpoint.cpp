#include "Interaction/ETCheckpoint.h"
#include "Character/ETPlayer.h"
#include "Controller/ETPlayerController.h"

AETCheckpoint::AETCheckpoint()
{
}

void AETCheckpoint::OnInteraction(AActor* InInteractor)
{
	Super::OnInteraction(InInteractor);

	AETPlayer* Player = Cast<AETPlayer>(InInteractor);
	if (Player == nullptr)
	{
		return;
	}

	Player->ResetGameStat();

	if (AETPlayerController* PlayerController = Cast<AETPlayerController>(Player->GetController()))
	{
		PlayerController->OpenCheckpointWidget();
	}

	// TODO : 체크포인트 저장
}
