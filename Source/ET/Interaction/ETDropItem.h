#pragma once

#include "CoreMinimal.h"
#include "Interaction/ETInteractionActor.h"
#include "ETDropItem.generated.h"

UCLASS()
class ET_API AETDropItem : public AETInteractionActor
{
	GENERATED_BODY()

public:
	AETDropItem();

public:
	// IETInteractionInterface
	virtual void OnInteraction(AActor* InInteractor) override;
	// ~IETInteractionInterface

protected:
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetDropItemRowNameOptions"))
	FName DropItemRowName;
};
