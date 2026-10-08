#include "Components/ETTimeFreezeComponent.h"
#include "Character/ETCharacter.h"
#include "Components/ETCharacterStateComponent.h"
#include "DataAsset/ETSkillTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"
#include "TimerManager.h"

UETTimeFreezeComponent::UETTimeFreezeComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UETTimeFreezeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UWorld* World = GetWorld())
	{
		World->GetTimerManager().ClearTimer(UnfreezeTimerHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void UETTimeFreezeComponent::CastTimeFreeze(const FName InRowName)
{
	const UETGameDataSubsystem* GameDataSubsystem = UGameInstance::GetSubsystem<UETGameDataSubsystem>(UGameplayStatics::GetGameInstance(this));
	const FETTimeFreezeTableRowData* RowData = GameDataSubsystem ? GameDataSubsystem->GetRow<FETTimeFreezeTableRowData>(InRowName) : nullptr;
	if (RowData == nullptr)
	{
		return;
	}

	TArray<AETCharacter*> TargetArray;
	FindTimeFreezeTargets(*RowData, TargetArray);

	for (AETCharacter* TargetCharacter : TargetArray)
	{
		if (UETTimeFreezeComponent* TargetTimeFreezeComponent = TargetCharacter->FindComponentByClass<UETTimeFreezeComponent>())
		{
			TargetTimeFreezeComponent->Freeze(RowData->FreezeDuration);
		}
	}
}

void UETTimeFreezeComponent::Freeze(const float InDuration)
{
	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr)
	{
		return;
	}

	if (bFrozen == false)
	{
		OriginCustomTimeDilation = OwnerActor->CustomTimeDilation;
		OwnerActor->CustomTimeDilation = 0.f;
		bFrozen = true;
	}

	// 액터 CustomTimeDilation 영향을 받지 않는 월드 타이머로 복구 (전체 TimeDilation 은 적용됨)
	FTimerManager& TimerManager = GetWorld()->GetTimerManager();
	TimerManager.ClearTimer(UnfreezeTimerHandle);

	if (InDuration > 0.f)
	{
		TimerManager.SetTimer(UnfreezeTimerHandle, this, &ThisClass::Unfreeze, InDuration, false);
	}
}

void UETTimeFreezeComponent::Unfreeze()
{
	GetWorld()->GetTimerManager().ClearTimer(UnfreezeTimerHandle);

	if (bFrozen == false)
	{
		return;
	}

	if (AActor* OwnerActor = GetOwner())
	{
		OwnerActor->CustomTimeDilation = OriginCustomTimeDilation;
	}

	bFrozen = false;
}

void UETTimeFreezeComponent::FindTimeFreezeTargets(const FETTimeFreezeTableRowData& InRowData, OUT TArray<AETCharacter*>& OutTargetArray) const
{
	OutTargetArray.Reset();

	AActor* OwnerActor = GetOwner();
	if (OwnerActor == nullptr || InRowData.SearchRadius <= 0.f)
	{
		return;
	}

	const FVector OwnerLocation = OwnerActor->GetActorLocation();
	const TArray<AActor*> IgnoreActorArray = { OwnerActor };
	TArray<AActor*> OverlapActorArray;

	UKismetSystemLibrary::SphereOverlapActors(OwnerActor, OwnerLocation, InRowData.SearchRadius, InRowData.ObjectTypeArray, AETCharacter::StaticClass(), IgnoreActorArray, OverlapActorArray);

	// 시야 기준은 카메라, 카메라가 없으면 Owner 정면
	const UCameraComponent* CameraComponent = OwnerActor->FindComponentByClass<UCameraComponent>();
	const FVector ViewLocation = CameraComponent ? CameraComponent->GetComponentLocation() : OwnerLocation;
	const FVector ViewDirection = CameraComponent ? CameraComponent->GetForwardVector() : OwnerActor->GetActorForwardVector();
	const float CosViewHalfAngle = FMath::Cos(FMath::DegreesToRadians(FMath::Clamp(InRowData.ViewHalfAngle, 0.f, 180.f)));

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

	if (InRowData.MaxTargetCount > 0 && OutTargetArray.Num() > InRowData.MaxTargetCount)
	{
		OutTargetArray.SetNum(InRowData.MaxTargetCount);
	}

#if ENABLE_DRAW_DEBUG
	if (InRowData.DrawDebugType != EDrawDebugTrace::None)
	{
		const bool bPersistent = InRowData.DrawDebugType == EDrawDebugTrace::Persistent;
		const float LifeTime = InRowData.DrawDebugType == EDrawDebugTrace::ForDuration ? InRowData.DrawDebugTime : 0.f;

		DrawDebugSphere(GetWorld(), OwnerLocation, InRowData.SearchRadius, 24, OutTargetArray.IsEmpty() ? FColor::Red : FColor::Green, bPersistent, LifeTime);
		for (const AETCharacter* TargetCharacter : OutTargetArray)
		{
			DrawDebugLine(GetWorld(), OwnerLocation, TargetCharacter->GetActorLocation(), FColor::Cyan, bPersistent, LifeTime);
		}
	}
#endif
}
