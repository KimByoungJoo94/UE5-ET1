#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ETAfterImageActor.generated.h"

class UPoseableMeshComponent;
class UMaterialInstanceDynamic;

UCLASS()
class ET_API AETAfterImageActor : public AActor
{
	GENERATED_BODY()

public:
	AETAfterImageActor();

public:
	virtual void Tick(float DeltaTime) override;

	void InitAfterImage(USkeletalMeshComponent* InSourceMeshComponent, UMaterialInterface* InMaterial, const float InLifeTime, const float InOpacity);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UPoseableMeshComponent> PoseableMeshComponent;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UMaterialInstanceDynamic>> MaterialInstanceArray;

private:
	float LifeTime = 0.f;
	float ElapsedTime = 0.f;
	float StartOpacity = 0.f;
};
