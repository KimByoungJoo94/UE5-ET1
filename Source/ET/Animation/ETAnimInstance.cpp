#include "Animation/ETAnimInstance.h"
#include "Character/ETCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

namespace
{
	const FName FullBodyCurveName = TEXT("FullBody");

	// Lean 세기 (클수록 약하게)
	constexpr float LeanIntensityScaling = 7.f;
	constexpr float YawDeltaInterpSpeed = 6.f;
}

UETAnimInstance::UETAnimInstance()
{
}

void UETAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

	OwnerCharacter = Cast<AETCharacter>(GetOwningActor());
	
	if (OwnerCharacter)
	{
		CharacterMovementComponent = OwnerCharacter->GetCharacterMovement();
		RotationLastTick = OwnerCharacter->GetActorRotation();
	}
}

void UETAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (OwnerCharacter == nullptr || CharacterMovementComponent == nullptr)
	{
		return;
	}

	const FVector Velocity = OwnerCharacter->GetVelocity();
	const FRotator ActorRotation = OwnerCharacter->GetActorRotation();

	Speed = Velocity.Size();
	bIsInAir = CharacterMovementComponent->IsFalling();
	bIsAccelerating = CharacterMovementComponent->GetCurrentAcceleration().Size() > 0.f;

	const FRotator AimRotation = UKismetMathLibrary::NormalizedDeltaRotator(OwnerCharacter->GetBaseAimRotation(), ActorRotation);
	Roll = AimRotation.Roll;
	Pitch = AimRotation.Pitch;
	Yaw = AimRotation.Yaw;

	if (FMath::IsNearlyZero(DeltaSeconds, UE_KINDA_SMALL_NUMBER) == false)
	{
		const FRotator RotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(RotationLastTick, ActorRotation);
		const float TargetYawDelta = RotationDelta.Yaw / DeltaSeconds / LeanIntensityScaling;
		YawDelta = FMath::FInterpTo(YawDelta, TargetYawDelta, DeltaSeconds, YawDeltaInterpSpeed);
	}
	RotationLastTick = ActorRotation;

	bIsFullbody = GetCurveValue(FullBodyCurveName) > 0.f;

	Direction = FMath::IsNearlyZero(Speed, UE_KINDA_SMALL_NUMBER) == false ? UKismetMathLibrary::NormalizedDeltaRotator(Velocity.Rotation(), ActorRotation).Yaw : 0.f;
	bStrafe = CharacterMovementComponent->bOrientRotationToMovement == false;
}
