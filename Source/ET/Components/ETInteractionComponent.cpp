#include "Components/ETInteractionComponent.h"
#include "Interface/ETInteractionInterface.h"
#include "Components/ETCharacterStateComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "Character/ETCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Animation/AnimInstance.h"
#include "Animation/AnimMontage.h"

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

bool UETInteractionComponent::StartInteraction()
{
	AETCharacter* OwnerCharacter = GetOwnerCharacter();
	if (OwnerCharacter == nullptr ||
		InteractionTarget.IsValid() == false ||
		OwnerCharacter->GetCharacterMovement()->IsFalling() ||
		OwnerCharacter->CanPlayAction(ETGameplayTags::Character_Action_Interaction) == false)
	{
		return false;
	}

	return OwnerCharacter->PlayAction(ETGameplayTags::Character_Action_Interaction, 0, ETGameplayTags::Character_State_Interacting);
}

void UETInteractionComponent::DoInteraction()
{
	DoInteraction(InteractionTarget.Get());
}

void UETInteractionComponent::DoInteraction(AActor* InTarget)
{
	IETInteractionInterface* InteractionInterface = Cast<IETInteractionInterface>(InTarget);
	if (IsValid(InTarget) == false || InteractionInterface == nullptr)
	{
		return;
	}

	InteractionInterface->OnInteraction(GetOwner());

	if (InteractionInterface->IsHoldInteraction())
	{
		if (UAnimInstance* AnimInstance = GetInteractingAnimInstance())
		{
			AnimInstance->Montage_Pause(AnimInstance->GetCurrentActiveMontage());
		}
	}
}

void UETInteractionComponent::EndInteraction()
{
	if (UAnimInstance* AnimInstance = GetInteractingAnimInstance())
	{
		AnimInstance->Montage_Resume(AnimInstance->GetCurrentActiveMontage());
	}
}

void UETInteractionComponent::CancelInteraction(const bool bInImmediately)
{
	if (UAnimInstance* AnimInstance = GetInteractingAnimInstance())
	{
		if (const UAnimMontage* CurrentMontage = AnimInstance->GetCurrentActiveMontage())
		{
			AnimInstance->Montage_Stop(bInImmediately ? 0.f : CurrentMontage->GetDefaultBlendOutTime(), CurrentMontage);
		}
	}
}

AETCharacter* UETInteractionComponent::GetOwnerCharacter() const
{
	return Cast<AETCharacter>(GetOwner());
}

UAnimInstance* UETInteractionComponent::GetInteractingAnimInstance() const
{
	// 대기 중 피격 등으로 다른 몽타주로 바뀐 경우 건드리지 않도록 상호작용 상태일 때만
	AETCharacter* OwnerCharacter = GetOwnerCharacter();
	if (OwnerCharacter == nullptr || OwnerCharacter->GetCharacterStateComponent()->IsCurrentState(ETGameplayTags::Character_State_Interacting) == false)
	{
		return nullptr;
	}

	return OwnerCharacter->GetMesh()->GetAnimInstance();
}
