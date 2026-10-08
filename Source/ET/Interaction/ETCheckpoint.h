#pragma once

#include "CoreMinimal.h"
#include "Interaction/ETInteractionActor.h"
#include "ETCheckpoint.generated.h"

UCLASS()
class ET_API AETCheckpoint : public AETInteractionActor
{
	GENERATED_BODY()

public:
	AETCheckpoint();

public:
	// IETInteractionInterface
	virtual void OnInteraction(AActor* InInteractor) override;
	// ~IETInteractionInterface
};
