#include "Animation/Notify/ETAnimNotify_AdvanceComboAttack.h"
#include "Character/ETPlayer.h"

void UETAnimNotify_AdvanceComboAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AETPlayer* PlayerCharacter = Cast<AETPlayer>(MeshComp->GetOwner()))
	{
		PlayerCharacter->AdvanceComboAttack();
	}
}