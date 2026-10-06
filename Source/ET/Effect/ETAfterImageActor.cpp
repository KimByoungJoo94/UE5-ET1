#include "Effect/ETAfterImageActor.h"
#include "Components/PoseableMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"

namespace
{
	const FName OpacityParameterName(TEXT("Opacity"));
}

AETAfterImageActor::AETAfterImageActor()
{
	PrimaryActorTick.bCanEverTick = true;

	PoseableMeshComponent = CreateDefaultSubobject<UPoseableMeshComponent>(TEXT("PoseableMeshComponent"));
	PoseableMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	PoseableMeshComponent->SetCastShadow(false);
	SetRootComponent(PoseableMeshComponent);
}

void AETAfterImageActor::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	ElapsedTime += DeltaTime;

	const float Opacity = StartOpacity * (1.f - FMath::Clamp(ElapsedTime / LifeTime, 0.f, 1.f));
	for (UMaterialInstanceDynamic* MaterialInstance : MaterialInstanceArray)
	{
		MaterialInstance->SetScalarParameterValue(OpacityParameterName, Opacity);
	}

	if (ElapsedTime >= LifeTime)
	{
		Destroy();
	}
}

void AETAfterImageActor::InitAfterImage(USkeletalMeshComponent* InSourceMeshComponent, UMaterialInterface* InMaterial, const float InLifeTime, const float InOpacity)
{
	if (IsValid(InSourceMeshComponent) == false || InMaterial == nullptr)
	{
		Destroy();
		return;
	}

	LifeTime = FMath::Max(InLifeTime, KINDA_SMALL_NUMBER);
	StartOpacity = InOpacity;

	PoseableMeshComponent->SetSkinnedAssetAndUpdate(InSourceMeshComponent->GetSkeletalMeshAsset());
	PoseableMeshComponent->SetWorldTransform(InSourceMeshComponent->GetComponentTransform());
	PoseableMeshComponent->CopyPoseFromSkeletalComponent(InSourceMeshComponent);

	MaterialInstanceArray.Reset();
	for (int32 Index = 0; Index < PoseableMeshComponent->GetNumMaterials(); ++Index)
	{
		UMaterialInstanceDynamic* MaterialInstance = UMaterialInstanceDynamic::Create(InMaterial, this);
		MaterialInstance->SetScalarParameterValue(OpacityParameterName, StartOpacity);

		PoseableMeshComponent->SetMaterial(Index, MaterialInstance);
		MaterialInstanceArray.Add(MaterialInstance);
	}
}
