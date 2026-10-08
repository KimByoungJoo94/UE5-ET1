#include "Animation/Notify/ETAnimNotify_Interaction.h"
#include "Character/ETPlayer.h"
#include "Components/ETInteractionComponent.h"

void UETAnimNotify_Interaction::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AETPlayer* PlayerCharacter = Cast<AETPlayer>(MeshComp->GetOwner()))
	{
		if (UETInteractionComponent* InteractionComponent = PlayerCharacter->GetInteractionComponent())
		{
			InteractionComponent->DoInteraction();
		}
	}
}
