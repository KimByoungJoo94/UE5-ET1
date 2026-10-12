#include "Components/ETLockOnComponent.h"
#include "Character/ETCharacter.h"
#include "Components/ETCharacterStateComponent.h"
#include "Interface/ETLockOnInterface.h"
#include "Common/ETGameplayTags.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Kismet/KismetMathLibrary.h"

UETLockOnComponent::UETLockOnComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UETLockOnComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	AETCharacter* Target = LockOnTarget.Get();
	if (Target == nullptr || IsTargetDead(Target))
	{
		EndLockOn();
		return;
	}

	UpdateControlRotation(DeltaTime);
	UpdateOwnerRotationMode();
}

void UETLockOnComponent::ToggleLockOn()
{
	if (IsLockedOn())
	{
		EndLockOn();
		return;
	}

	if (AETCharacter* Target = FindLockOnTarget())
	{
		StartLockOn(Target);
	}
}

void UETLockOnComponent::StartLockOn(AETCharacter* InTarget)
{
	IETLockOnInterface* LockOnInterface = Cast<IETLockOnInterface>(InTarget);
	if (LockOnInterface == nullptr)
	{
		return;
	}

	EndLockOn();

	LockOnTarget = InTarget;
	LockOnInterface->OnLockOnTargeted();

	SetStrafeMode(true);
	SetComponentTickEnabled(true);
}

void UETLockOnComponent::EndLockOn()
{
	if (IETLockOnInterface* LockOnInterface = Cast<IETLockOnInterface>(LockOnTarget.Get()))
	{
		LockOnInterface->OnLockOnTargetReleased();
	}

	LockOnTarget.Reset();

	SetStrafeMode(false);
	SetComponentTickEnabled(false);
}

AETCharacter* UETLockOnComponent::GetLockOnTarget() const
{
	return LockOnTarget.Get();
}

AETCharacter* UETLockOnComponent::FindLockOnTarget() const
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;
	if (OwnerController == nullptr)
	{
		return nullptr;
	}

	// 플레이어 컨트롤러면 카메라 시점
	FVector ViewLocation;
	FRotator ViewRotation;
	OwnerController->GetPlayerViewPoint(ViewLocation, ViewRotation);
	const FVector ViewDirection = ViewRotation.Vector();
	const FVector OwnerLocation = OwnerPawn->GetActorLocation();

	const TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeArray = { UEngineTypes::ConvertToObjectType(ECC_Pawn) };
	const TArray<AActor*> IgnoreActorArray = { OwnerPawn };
	TArray<AActor*> OverlapActorArray;
	UKismetSystemLibrary::SphereOverlapActors(OwnerPawn, OwnerLocation, SearchRadius, ObjectTypeArray, AETCharacter::StaticClass(), IgnoreActorArray, OverlapActorArray);

	const float CosViewHalfAngle = FMath::Cos(FMath::DegreesToRadians(ViewHalfAngle));
	const float SafeSearchRadius = FMath::Max(SearchRadius, KINDA_SMALL_NUMBER);

	AETCharacter* BestTarget = nullptr;
	float BestScore = TNumericLimits<float>::Max();

	for (AActor* OverlapActor : OverlapActorArray)
	{
		AETCharacter* Candidate = Cast<AETCharacter>(OverlapActor);
		if (Candidate == nullptr || Candidate->Implements<UETLockOnInterface>() == false || IsTargetDead(Candidate))
		{
			continue;
		}

		const FVector TargetLocation = GetTargetLookAtLocation(Candidate);
		const float CosAngle = FVector::DotProduct(ViewDirection, (TargetLocation - ViewLocation).GetSafeNormal());
		if (CosAngle < CosViewHalfAngle)
		{
			continue;
		}

		// 카메라와 대상 사이가 막혀 있으면 제외
		FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ETLockOn), false);
		QueryParams.AddIgnoredActor(OwnerPawn);
		QueryParams.AddIgnoredActor(Candidate);

		FHitResult HitResult;
		if (GetWorld()->LineTraceSingleByChannel(HitResult, ViewLocation, TargetLocation, ECC_Visibility, QueryParams))
		{
			continue;
		}

		const float AngleRatio = FMath::RadiansToDegrees(FMath::Acos(FMath::Clamp(CosAngle, -1.f, 1.f))) / ViewHalfAngle;
		const float DistanceRatio = FVector::Dist(OwnerLocation, Candidate->GetActorLocation()) / SafeSearchRadius;
		const float Score = AngleRatio * AngleWeight + DistanceRatio * (1.f - AngleWeight);

		if (Score < BestScore)
		{
			BestScore = Score;
			BestTarget = Candidate;
		}
	}

	return BestTarget;
}

bool UETLockOnComponent::IsTargetDead(AETCharacter* InTarget) const
{
	const UETCharacterStateComponent* TargetStateComponent = InTarget->GetCharacterStateComponent();
	return TargetStateComponent && TargetStateComponent->IsCurrentState(ETGameplayTags::Character_State_Death);
}

FVector UETLockOnComponent::GetTargetLookAtLocation(const AETCharacter* InTarget) const
{
	return InTarget->GetActorLocation() + FVector(0.f, 0.f, TargetHeightOffset);
}

void UETLockOnComponent::UpdateControlRotation(const float InDeltaTime)
{
	APawn* OwnerPawn = Cast<APawn>(GetOwner());
	AController* OwnerController = OwnerPawn ? OwnerPawn->GetController() : nullptr;
	if (OwnerController == nullptr)
	{
		return;
	}

	// 카메라가 아닌 Owner 기준 고정 지점에서 계산해야 카메라 이동에 흔들리지 않음
	const FVector ViewPivotLocation = OwnerPawn->GetActorLocation() + FVector(0.f, 0.f, ViewPivotHeightOffset);
	FRotator TargetRotation = UKismetMathLibrary::FindLookAtRotation(ViewPivotLocation, GetTargetLookAtLocation(LockOnTarget.Get()));
	TargetRotation.Pitch = FMath::Clamp(TargetRotation.Pitch + PitchOffset, MinPitch, MaxPitch);
	TargetRotation.Roll = 0.f;

	OwnerController->SetControlRotation(FMath::RInterpTo(OwnerController->GetControlRotation(), TargetRotation, InDeltaTime, RotationInterpSpeed));
}

void UETLockOnComponent::UpdateOwnerRotationMode()
{
	AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	// 회피는 입력 방향으로 돌아서 굴러야 하므로 회피 중에는 대상 방향 회전을 끔
	const bool bDodging = OwnerCharacter->GetCharacterStateComponent()->IsCurrentState(ETGameplayTags::Character_State_Dodging);
	OwnerCharacter->GetCharacterMovement()->bUseControllerDesiredRotation = bDodging == false;
}

void UETLockOnComponent::SetStrafeMode(const bool bInStrafe)
{
	AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner());
	if (OwnerCharacter == nullptr)
	{
		return;
	}

	// 컨트롤 회전이 대상을 향하므로 컨트롤 회전을 따라 돌면 대상을 바라본 채 옆걸음
	if (UCharacterMovementComponent* CharacterMovementComponent = OwnerCharacter->GetCharacterMovement())
	{
		CharacterMovementComponent->bOrientRotationToMovement = bInStrafe == false;
		CharacterMovementComponent->bUseControllerDesiredRotation = bInStrafe;
	}
}
