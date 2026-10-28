#include "Animation/Notify/ETAnimNotify_CameraShake.h"
#include "Subsystem/ETJobSubsystem.h"
#include "Job/ETCameraJobs.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/Pawn.h"

void UETAnimNotify_CameraShake::Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::Notify(MeshComp, Animation, EventReference);

	// 에디터 프리뷰 등 게임 월드가 아니면 재생하지 않음
	AActor* OwnerActor = MeshComp ? MeshComp->GetOwner() : nullptr;
	UWorld* World = OwnerActor ? OwnerActor->GetWorld() : nullptr;
	if (World == nullptr || World->IsGameWorld() == false || CameraShakeRowName.IsNone())
	{
		return;
	}

	const APawn* OwnerPawn = Cast<APawn>(OwnerActor);
	TUniquePtr<FETCameraShakeJob> CameraShakeJob = OwnerPawn && OwnerPawn->IsLocallyControlled() && OwnerPawn->IsPlayerControlled()
		? MakeUnique<FETCameraShakeJob>(CameraShakeRowName)
		: MakeUnique<FETCameraShakeJob>(CameraShakeRowName, OwnerActor->GetActorLocation());

	ETSubsystem::Get<UETJobSubsystem>(World).Run(OwnerActor, MoveTemp(CameraShakeJob));
}
