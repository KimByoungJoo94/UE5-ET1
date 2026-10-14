#include "Interaction/ETDropItem.h"
#include "Character/ETPlayer.h"
#include "DataAsset/ETDropItemTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"

AETDropItem::AETDropItem()
{
}

void AETDropItem::OnInteraction(AActor* InInteractor)
{
	Super::OnInteraction(InInteractor);

	AETPlayer* Player = Cast<AETPlayer>(InInteractor);
	if (Player == nullptr)
	{
		return;
	}

	const FETDropItemTableRowData* DropItemRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETDropItemTableRowData>(DropItemRowName);
	if (DropItemRowData == nullptr)
	{
		return;
	}

	for (const TPair<EETGameStatType, float>& PermanentGameStatPair : DropItemRowData->PermanentGameStatMap)
	{
		Player->AddPermanentGameStat(PermanentGameStatPair.Key, PermanentGameStatPair.Value);
	}

	Destroy();
}
