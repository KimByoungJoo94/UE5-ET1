#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ETDissolveComponent.generated.h"

class UMaterialInstanceDynamic;

DECLARE_MULTICAST_DELEGATE(FOnDissolveFinished);

// Owner 캐릭터 메시의 모든 슬롯을 디졸브 머터리얼로 교체 후 게임 시간 기준으로 진행
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETDissolveComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UETDissolveComponent();

public:
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void StartDissolve();

	FORCEINLINE bool IsDissolving() const { return DissolveMaterialInstance != nullptr; }

public:
	FOnDissolveFinished OnDissolveFinished;

protected:
	UPROPERTY(EditAnywhere, Category = "Dissolve", meta = (ClampMin = "0.01"))
	float DissolveDuration = 3.0f;

	UPROPERTY(EditAnywhere, Category = "Dissolve")
	float DissolveStartValue = 1.f;

	UPROPERTY(EditAnywhere, Category = "Dissolve")
	float DissolveEndValue = -1.f;

private:
	// 모든 슬롯이 공유
	UPROPERTY(Transient)
	TObjectPtr<UMaterialInstanceDynamic> DissolveMaterialInstance;

	float DissolveElapsedTime = 0.f;
};
