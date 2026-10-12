#include "Components/ETTimeFreezeComponent.h"
#include "Character/ETCharacter.h"
#include "Components/ETCharacterStateComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DataAsset/ETSkillTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Common/ETCommonSettings.h"
#include "Materials/MaterialInterface.h"
#include "Camera/CameraComponent.h"
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
	const FETTimeFreezeTableRowData* RowData = UETGameDataSubsystem::GetRow<FETTimeFreezeTableRowData>(this, InRowName);
	if (RowData == nullptr)
	{
		return;
	}

	TArray<AETCharacter*> TargetArray;
	FindTimeFreezeTargets(*RowData, TargetArray);

	for (AETCharacter* TargetCharacter : TargetArray)
	{
		if (UETTimeFreezeComponent* TargetTimeFreezeComponent = TargetCharacter->GetTimeFreezeComponent())
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

		ApplyTimeFreezeMaterial();
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

	RestoreOriginMaterials();

	bFrozen = false;
}

void UETTimeFreezeComponent::ApplyTimeFreezeMaterial()
{
	AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner());
	USkeletalMeshComponent* MeshComponent = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
	UMaterialInterface* TimeFreezeMaterial = GetDefault<UETCommonSettings>()->Character.TimeFreezeMaterial.LoadSynchronous();
	if (MeshComponent == nullptr || TimeFreezeMaterial == nullptr)
	{
		return;
	}

	const int32 MaterialNum = MeshComponent->GetNumMaterials();

	OriginMaterialArray.Reset(MaterialNum);
	for (int32 Index = 0; Index < MaterialNum; ++Index)
	{
		OriginMaterialArray.Add(MeshComponent->GetMaterial(Index));
		MeshComponent->SetMaterial(Index, TimeFreezeMaterial);
	}
}

void UETTimeFreezeComponent::RestoreOriginMaterials()
{
	AETCharacter* OwnerCharacter = Cast<AETCharacter>(GetOwner());
	if (USkeletalMeshComponent* MeshComponent = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr)
	{
		for (int32 Index = 0; Index < OriginMaterialArray.Num(); ++Index)
		{
			MeshComponent->SetMaterial(Index, OriginMaterialArray[Index]);
		}
	}

	OriginMaterialArray.Reset();
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

	// 락온 대상이 조건을 통과했으면 거리와 상관없이 가장 앞으로 (대상 수 제한에도 우선 포함)
	if (const AETCharacter* OwnerCharacter = Cast<AETCharacter>(OwnerActor))
	{
		AETCharacter* LockOnTarget = OwnerCharacter->GetLockOnTarget();
		if (LockOnTarget && OutTargetArray.Remove(LockOnTarget) > 0)
		{
			OutTargetArray.Insert(LockOnTarget, 0);
		}
	}

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
