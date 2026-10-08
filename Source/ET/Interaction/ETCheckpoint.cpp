#include "Interaction/ETCheckpoint.h"

AETCheckpoint::AETCheckpoint()
{
}

void AETCheckpoint::OnInteraction(AActor* InInteractor)
{
	Super::OnInteraction(InInteractor);

	// TODO : 플레이어 회복, 체크포인트 저장
}
