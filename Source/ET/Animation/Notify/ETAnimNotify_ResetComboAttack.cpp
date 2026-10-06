#include "Animation/Notify/ETAnimNotify_ResetComboAttack.h"
#include "Character/ETPlayer.h"

void UETAnimNotify_ResetComboAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AETPlayer* PlayerCharacter = Cast<AETPlayer>(MeshComp->GetOwner()))
	{
		PlayerCharacter->ResetComboAttack();
	}
}
