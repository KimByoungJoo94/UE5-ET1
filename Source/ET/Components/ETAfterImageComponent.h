#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ETAfterImageComponent.generated.h"

class USkeletalMeshComponent;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETAfterImageComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UETAfterImageComponent();

public:
	// InSourceMeshComponent 가 nullptr 이면 소유 캐릭터의 Mesh 사용
	void SpawnAfterImage(USkeletalMeshComponent* InSourceMeshComponent = nullptr);

protected:
	UPROPERTY(EditAnywhere, Category = "AfterImage", meta = (ClampMin = "0.01"))
	float AfterImageLifeTime = 0.5f;

	UPROPERTY(EditAnywhere, Category = "AfterImage", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AfterImageOpacity = 0.5f;
};
