#include "Components/ETInteractionComponent.h"
#include "Interface/ETInteractionInterface.h"

UETInteractionComponent::UETInteractionComponent()
{	
	PrimaryComponentTick.bCanEverTick = false;
}

void UETInteractionComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UETInteractionComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UETInteractionComponent::SetInteractionTarget(AActor* InTarget)
{
	InteractionTarget = InTarget;
}

void UETInteractionComponent::ClearInteractionTarget()
{
	InteractionTarget = nullptr;
}

void UETInteractionComponent::DoInteraction()
{
	if (IsValid(InteractionTarget))
	{
		if (IETInteractionInterface* InteractionInterface = Cast<IETInteractionInterface>(InteractionTarget))
		{
			InteractionInterface->OnInteraction(GetOwner());
		}
	}
}

