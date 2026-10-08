#include "Components/ETAfterImageComponent.h"
#include "GameFramework/Character.h"
#include "Components/SkeletalMeshComponent.h"
#include "Effect/ETAfterImageActor.h"
#include "Common/ETCommonSettings.h"
#include "Materials/MaterialInterface.h"

UETAfterImageComponent::UETAfterImageComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UETAfterImageComponent::SpawnAfterImage(USkeletalMeshComponent* InSourceMeshComponent)
{
	USkeletalMeshComponent* SourceMeshComponent = InSourceMeshComponent;
	if (SourceMeshComponent == nullptr)
	{
		if (ACharacter* OwnerCharacter = Cast<ACharacter>(GetOwner()))
		{
			SourceMeshComponent = OwnerCharacter->GetMesh();
		}
	}

	UMaterialInterface* AfterImageMaterial = GetDefault<UETCommonSettings>()->Character.AfterImageMaterial.LoadSynchronous();
	if (IsValid(SourceMeshComponent) == false || AfterImageMaterial == nullptr)
	{
		return;
	}

	FActorSpawnParameters SpawnParameters;
	SpawnParameters.Owner = GetOwner();
	SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	if (AETAfterImageActor* AfterImageActor = GetWorld()->SpawnActor<AETAfterImageActor>(AETAfterImageActor::StaticClass(), SourceMeshComponent->GetComponentTransform(), SpawnParameters))
	{
		AfterImageActor->InitAfterImage(SourceMeshComponent, AfterImageMaterial, AfterImageLifeTime, AfterImageOpacity);
	}
}
