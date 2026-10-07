#include "Animation/Notify/ETAnimNotify_PauseParry.h"
#include "Character/ETPlayer.h"

void UETAnimNotify_PauseParry::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AETPlayer* PlayerCharacter = Cast<AETPlayer>(MeshComp->GetOwner()))
	{
		PlayerCharacter->PauseParry();
	}
}
