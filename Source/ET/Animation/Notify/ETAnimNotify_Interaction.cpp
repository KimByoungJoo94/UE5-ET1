#include "Animation/Notify/ETAnimNotify_Interaction.h"
#include "Components/ETInteractionComponent.h"

void UETAnimNotify_Interaction::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (UETInteractionComponent* InteractionComponent = MeshComp->GetOwner()->FindComponentByClass<UETInteractionComponent>())
	{
		InteractionComponent->DoInteraction();
	}
}
