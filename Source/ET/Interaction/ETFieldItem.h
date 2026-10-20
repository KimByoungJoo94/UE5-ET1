#pragma once

#include "CoreMinimal.h"
#include "Interaction/ETInteractionActor.h"
#include "ETFieldItem.generated.h"

class UParticleSystemComponent;
struct FETFieldItemTableRowData;

UCLASS()
class ET_API AETFieldItem : public AETInteractionActor
{
	GENERATED_BODY()

public:
	AETFieldItem();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;

public:
	// IETInteractionInterface
	virtual void OnInteraction(AActor* InInteractor) override;
	// ~IETInteractionInterface

private:
	void ApplyFieldItemRowData(const FETFieldItemTableRowData* InFieldItemRowData);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UParticleSystemComponent> ParticleSystemComponent;

	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetFieldItemRowNameOptions"))
	FName FieldItemRowName;
};
