#include "Animation/ETAnimInstance.h"
#include "Character/ETCharacter.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UETAnimInstance::UETAnimInstance()
{
}

void UETAnimInstance::NativeInitializeAnimation()
{
	Super::NativeInitializeAnimation();

    Character = Cast<AETCharacter>(GetOwningActor());
}

void UETAnimInstance::NativeUpdateAnimation(float DeltaSeconds)
{
	Super::NativeUpdateAnimation(DeltaSeconds);

	if (Character)
	{
		Speed = Character->GetVelocity().Length();

		{
			const FRotator& CurrentRotation = Character->GetActorRotation();
			const FRotator& RotationDelta = UKismetMathLibrary::NormalizedDeltaRotator(RotationLastTick, CurrentRotation);
			const float TargetYawDelta = Speed > KINDA_SMALL_NUMBER ? RotationDelta.Yaw / Speed / 7.f : 0.f;
			
			YawDelta = FMath::FInterpTo(YawDelta, TargetYawDelta, DeltaSeconds, 6.f);
			RotationLastTick = CurrentRotation;
		}		

		if (UCharacterMovementComponent* CharacterMovementComponent = Character->GetCharacterMovement())
		{
			bInAir = CharacterMovementComponent->IsFalling();
			bAccelerating = CharacterMovementComponent->GetCurrentAcceleration().Length() > 0.f;
		}
	}
}
