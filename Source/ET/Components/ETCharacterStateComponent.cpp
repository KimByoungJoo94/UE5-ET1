#include "Components/ETCharacterStateComponent.h"

UETCharacterStateComponent::UETCharacterStateComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UETCharacterStateComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UETCharacterStateComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UETCharacterStateComponent::ChangeState(const FGameplayTag InNewState)
{
	if (CurrentState != ETGameplayTags::Character_State_Death &&
		CurrentState != InNewState)
	{
		CurrentState = InNewState;
	}
}

void UETCharacterStateComponent::ClearState()
{
	if (CurrentState != ETGameplayTags::Character_State_Death)
	{
		CurrentState = FGameplayTag::EmptyTag;
	}
}