#include "Components/ETLockOnComponent.h"
#include "Character/ETCharacter.h"
#include "Components/ETCharacterStateComponent.h"
#include "Camera/CameraComponent.h"
#include "DrawDebugHelpers.h"

UETLockOnComponent::UETLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = false;

	ObjectTypeArray.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
}

AETCharacter* UETLockOnComponent::GetLockOnTarget() const
{
	return nullptr;
}

void UETLockOnComponent::FindTargetsInView(const float InRadius, const float InViewHalfAngle, const int32 InMaxCount, OUT TArray<AETCharacter*>& OutTargetArray) const
{
	OutTargetArray.Reset();

	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr || InRadius <= 0.f)
	{
		return;
	}

	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const TArray<AActor*> IgnoreActorArray = { OwnerActor };
	TArray<AActor*> OverlapActorArray;

	UKismetSystemLibrary::SphereOverlapActors(OwnerActor, OwnerLocation, InRadius, ObjectTypeArray, AETCharacter::StaticClass(), IgnoreActorArray, OverlapActorArray);

	// 카메라가 없으면 오너 정면 기준
	const UCameraComponent* CameraComponent = OwnerActor->FindComponentByClass<UCameraComponent>();
	const FVector ViewLocation = CameraComponent ? CameraComponent->GetComponentLocation() : OwnerLocation;
	const FVector ViewDirection = CameraComponent ? CameraComponent->GetForwardVector() : OwnerActor->GetActorForwardVector();
	const float CosViewHalfAngle = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(InViewHalfAngle, 0.f, 180.f)));

	for (AActor* OverlapActor : OverlapActorArray)
	{
		AETCharacter* OverlapCharacter = Cast<AETCharacter>(OverlapActor);
		if (OverlapCharacter == nullptr || OverlapCharacter->GetCharacterStateComponent()->IsCurrentState(ETGameplayTags::Character_State_Death))
		{
			continue;
		}

		const FVector ToCharacter = (OverlapCharacter->GetActorLocation() - ViewLocation).GetSafeNormal();
		if (FVector::DotProduct(ViewDirection, ToCharacter) >= CosViewHalfAngle)
		{
			OutTargetArray.Add(OverlapCharacter);
		}
	}

	OutTargetArray.Sort([&OwnerLocation](const AETCharacter& InLeft, const AETCharacter& InRight)
	{
		return FVector::DistSquared(OwnerLocation, InLeft.GetActorLocation()) < FVector::DistSquared(OwnerLocation, InRight.GetActorLocation());
	});

	if (InMaxCount > 0 && OutTargetArray.Num() > InMaxCount)
	{
		OutTargetArray.SetNum(InMaxCount);
	}

#if ENABLE_DRAW_DEBUG
	if (DrawDebugType != EDrawDebugTrace::None)
	{
		const bool bPersistent = DrawDebugType == EDrawDebugTrace::Persistent;
		const float LifeTime = DrawDebugType == EDrawDebugTrace::ForDuration ? DrawDebugTime : 0.f;

		DrawDebugSphere(GetWorld(), OwnerLocation, InRadius, 24, OutTargetArray.IsEmpty() ? FColor::Red : FColor::Green, bPersistent, LifeTime);
		for (const AETCharacter* TargetCharacter : OutTargetArray)
		{
			DrawDebugLine(GetWorld(), OwnerLocation, TargetCharacter->GetActorLocation(), FColor::Cyan, bPersistent, LifeTime);
		}
	}
#endif
}
