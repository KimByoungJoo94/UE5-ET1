#include "Animation/Notify/ETAnimNotify_ResetHeavyAttack.h"
#include "Character/ETPlayer.h"

void UETAnimNotify_ResetHeavyAttack::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (AETPlayer* PlayerCharacter = Cast<AETPlayer>(MeshComp->GetOwner()))
	{
		PlayerCharacter->ResetHeavyAttack();
	}
}