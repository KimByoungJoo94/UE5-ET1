#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ETInteractionComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UETInteractionComponent();

protected:	
	virtual void BeginPlay() override;

public:		
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	void SetInteractionTarget(AActor* InTarget);
	void ClearInteractionTarget();
	void DoInteraction();

private:
	UPROPERTY(Transient)
	TObjectPtr<AActor> InteractionTarget;
};
