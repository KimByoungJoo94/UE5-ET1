#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ETCombatComponent.generated.h"

class UETCharacterActionDataAsset;

USTRUCT()
struct FETWeaponCollisionData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	FName StartSocketName;

	UPROPERTY(EditAnywhere)
	FName EndSocketName;
	
	UPROPERTY(EditAnywhere)
	float Radius = 20.f;
		
	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypes;
		
	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::ForDuration;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UETCombatComponent();

protected:	
	virtual void BeginPlay() override;

public:		
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
	FORCEINLINE bool IsWeaponCollisionTracing() { return bWeaponCollisionTracing; }
	void StartWeaponCollision();
	void EndWeaponCollision();
	
protected:
	void TraceWeaponCollision();

protected:
	UPROPERTY(EditAnywhere)
	FETWeaponCollisionData WeaponCollisionData;
	
	TSet<TWeakObjectPtr<AActor>> AlreadHitActorSet;

	bool bWeaponCollisionTracing = false;
};
