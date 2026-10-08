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
	if (InteractionTarget.Get() != InTarget)
	{
		ReleaseInteractionTarget();
	}

	InteractionTarget = InTarget;
}

void UETInteractionComponent::ClearInteractionTarget()
{
	ReleaseInteractionTarget();

	InteractionTarget = nullptr;
}

void UETInteractionComponent::ReleaseInteractionTarget()
{
	if (IETInteractionInterface* InteractionInterface = Cast<IETInteractionInterface>(InteractionTarget.Get()))
	{
		InteractionInterface->OnInteractionTargetReleased();
	}
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

