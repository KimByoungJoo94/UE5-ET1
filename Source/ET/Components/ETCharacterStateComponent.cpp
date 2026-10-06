#include "Components/ETCharacterStateComponent.h"
#include "Engine/Engine.h"

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
		const FGameplayTag PrevState = CurrentState;
		CurrentState = InNewState;

		const FString Message = FString::Printf(TEXT("[%s] ChangeState: %s -> %s"), *GetNameSafe(GetOwner()), *PrevState.ToString(), *CurrentState.ToString());

		UE_LOG(LogTemp, Log, TEXT("%s"), *Message);

#if !UE_BUILD_SHIPPING
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 2.f, FColor::Yellow, Message);
		}
#endif
	}
}

void UETCharacterStateComponent::ClearState()
{
	if (CurrentState != ETGameplayTags::Character_State_Death)
	{
		CurrentState = FGameplayTag::EmptyTag;
	}
}