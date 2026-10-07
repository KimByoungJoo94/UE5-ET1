#include "Animation/Notify/ETAnimNotifyState_CapsuleHalfHeight.h"
#include "Animation/AnimNotifyLibrary.h"
#include "Character/ETCharacter.h"
#include "Components/CapsuleComponent.h"

void UETAnimNotifyState_CapsuleHalfHeight::NotifyTick(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float FrameDeltaTime, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyTick(MeshComp, Animation, FrameDeltaTime, EventReference);

	AETCharacter* OwnerCharacter = Cast<AETCharacter>(MeshComp->GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	// 줄어드는 구간과 복구 구간이 겹치지 않도록 보정
	const float ClampedShrinkRatio = FMath::Min(ShrinkRatio, 1.f);
	const float ClampedRestoreRatio = FMath::Min(RestoreRatio, 1.f - ClampedShrinkRatio);

	const float TimeRatio = UAnimNotifyLibrary::GetCurrentAnimationNotifyStateTimeRatio(EventReference);

	// 0 : 기본 높이, 1 : 목표 높이
	float Alpha = 1.f;
	if (TimeRatio < ClampedShrinkRatio)
	{
		Alpha = TimeRatio / ClampedShrinkRatio;
	}
	else if (ClampedRestoreRatio > 0.f && TimeRatio > 1.f - ClampedRestoreRatio)
	{
		Alpha = (1.f - TimeRatio) / ClampedRestoreRatio;
	}

	Alpha = FAlphaBlend::AlphaToBlendOption(FMath::Clamp(Alpha, 0.f, 1.f), BlendOption);

	const float HalfHeight = FMath::Lerp(OwnerCharacter->GetDefaultCapsuleHalfHeight(), TargetHalfHeight, Alpha);
	OwnerCharacter->SetCapsuleHalfHeightKeepGround(HalfHeight);
}

void UETAnimNotifyState_CapsuleHalfHeight::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
	Super::NotifyEnd(MeshComp, Animation, EventReference);

	if (AETCharacter* OwnerCharacter = Cast<AETCharacter>(MeshComp->GetOwner()))
	{
		OwnerCharacter->ResetCapsuleHalfHeight();
	}
}
