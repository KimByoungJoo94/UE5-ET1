#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Interface/ETInteractionInterface.h"
#include "ETInteractionActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;
class UWidgetComponent;
class UArrowComponent;

// 상호작용 액터 공통 (박스 오버랩 시 플레이어 상호작용 대상 등록, 키 위젯 표시)
UCLASS(Abstract)
class ET_API AETInteractionActor : public AActor, public IETInteractionInterface
{
	GENERATED_BODY()

public:
	AETInteractionActor();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	UFUNCTION()
	void OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnInteractionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

public:
	// IETInteractionInterface
	virtual void OnInteraction(AActor* InInteractor) override;
	virtual void OnInteractionTargetReleased() override;
	// ~IETInteractionInterface

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> RootSceneComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> StaticMeshComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> InteractionBoxComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UWidgetComponent> InteractionWidgetComponent;

#if WITH_EDITORONLY_DATA
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UArrowComponent> ArrowComponent;
#endif
};
