#include "Animation/Notify/ETAnimNotify_Dissolve.h"
#include "Components/ETDissolveComponent.h"
#include "Components/SkeletalMeshComponent.h"

void UETAnimNotify_Dissolve::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// 디졸브 컴포넌트를 가진 캐릭터만 (몬스터)
	if (AActor* OwnerActor = MeshComp->GetOwner())
	{
		if (UETDissolveComponent* DissolveComponent = OwnerActor->FindComponentByClass<UETDissolveComponent>())
		{
			DissolveComponent->StartDissolve();
		}
	}
}
