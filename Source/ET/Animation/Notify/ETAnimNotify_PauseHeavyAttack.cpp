#include "Animation/Notify/ETAnimNotify_PauseHeavyAttack.h"
#include "Character/ETPlayer.h"

void UETAnimNotify_PauseHeavyAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AETPlayer* PlayerCharacter = Cast<AETPlayer>(MeshComp->GetOwner()))
	{
		PlayerCharacter->PauseHeavyAttack();
	}
}