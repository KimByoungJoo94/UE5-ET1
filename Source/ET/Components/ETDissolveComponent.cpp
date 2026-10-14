#include "Components/ETDissolveComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Common/ETCommonSettings.h"
#include "Common/ETNames.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

UETDissolveComponent::UETDissolveComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
	PrimaryComponentTick.bStartWithTickEnabled = false;
}

void UETDissolveComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// DeltaTime 에 전체 TimeDilation 과 Owner 의 CustomTimeDilation 이 반영되어 있음 (게임 시간 기준)
	DissolveElapsedTime += DeltaTime;

	const float Alpha = FMath::Clamp(DissolveElapsedTime / DissolveDuration, 0.f, 1.f);
	DissolveMaterialInstance->SetScalarParameterValue(ETNames::MaterialParameter::Dissolve, FMath::Lerp(DissolveStartValue, DissolveEndValue, Alpha));

	if (Alpha >= 1.f)
	{
		SetComponentTickEnabled(false);
		OnDissolveFinished.Broadcast();
	}
}

void UETDissolveComponent::StartDissolve()
{
	if (IsDissolving())
	{
		return;
	}

	const ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner());
	USkeletalMeshComponent* MeshComponent = OwnerCharacter ? OwnerCharacter->GetMesh() : nullptr;
	UMaterialInterface* DissolveMaterial = GetDefault<UETCommonSettings>()->Character.DissolveMaterial.LoadSynchronous();
	if (MeshComponent == nullptr || DissolveMaterial == nullptr)
	{
		// 디졸브 없이 바로 완료 처리 (사라짐 처리는 그대로 진행)
		OnDissolveFinished.Broadcast();
		return;
	}

	DissolveMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterial, this);
	DissolveMaterialInstance->SetScalarParameterValue(ETNames::MaterialParameter::Dissolve, DissolveStartValue);

	for (int32 Index = 0; Index < MeshComponent->GetNumMaterials(); ++Index)
	{
		MeshComponent->SetMaterial(Index, DissolveMaterialInstance);
	}

	DissolveElapsedTime = 0.f;
	SetComponentTickEnabled(true);
}
