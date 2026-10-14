#include "Animation/Notify/ETAnimNotify_Defeat.h"
#include "Interface/ETDefeatInterface.h"
#include "Components/SkeletalMeshComponent.h"

void UETAnimNotify_Defeat::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	if (IETDefeatInterface* DefeatInterface = Cast<IETDefeatInterface>(MeshComp->GetOwner()))
	{
		DefeatInterface->OnDefeated();
	}
}
