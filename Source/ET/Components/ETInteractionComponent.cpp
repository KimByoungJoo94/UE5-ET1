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
	DoInteraction(InteractionTarget.Get());
}

void UETInteractionComponent::DoInteraction(AActor* InTarget)
{
	if (IsValid(InTarget))
	{
		if (IETInteractionInterface* InteractionInterface = Cast<IETInteractionInterface>(InTarget))
		{
			InteractionInterface->OnInteraction(GetOwner());
		}
	}
}

