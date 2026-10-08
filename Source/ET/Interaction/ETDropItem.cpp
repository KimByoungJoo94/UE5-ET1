#include "Interaction/ETDropItem.h"
#include "Character/ETCharacter.h"
#include "Components/ETGameStatComponent.h"
#include "DataAsset/ETDropItemTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Kismet/GameplayStatics.h"

AETDropItem::AETDropItem()
{
}

void AETDropItem::OnInteraction(AActor* InInteractor)
{
	Super::OnInteraction(InInteractor);

	AETCharacter* InteractorCharacter = Cast<AETCharacter>(InInteractor);
	if (InteractorCharacter == nullptr)
	{
		return;
	}

	const UETGameDataSubsystem* GameDataSubsystem = UGameInstance::GetSubsystem<UETGameDataSubsystem>(UGameplayStatics::GetGameInstance(this));
	const FETDropItemTableRowData* DropItemRowData = GameDataSubsystem ? GameDataSubsystem->GetRow<FETDropItemTableRowData>(DropItemRowName) : nullptr;
	if (DropItemRowData == nullptr)
	{
		return;
	}

	UETGameStatComponent* GameStatComponent = InteractorCharacter->GetGameStatComponent();
	for (const TPair<EETGameStatType, float>& PermanentGameStatPair : DropItemRowData->PermanentGameStatMap)
	{
		GameStatComponent->AddPermanentValue(PermanentGameStatPair.Key, PermanentGameStatPair.Value);
	}

	Destroy();
}
