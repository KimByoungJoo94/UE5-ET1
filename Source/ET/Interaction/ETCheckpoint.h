#pragma once

#include "CoreMinimal.h"
#include "Interaction/ETInteractionActor.h"
#include "ETCheckpoint.generated.h"

UCLASS()
class ET_API AETCheckpoint : public AETInteractionActor
{
	GENERATED_BODY()

public:
	AETCheckpoint();

public:
	// IETInteractionInterface
	virtual void OnInteraction(AActor* InInteractor) override;
	virtual bool IsHoldInteraction() const override { return true; }
	// ~IETInteractionInterface

public:
	// 플레이어 부활 위치 (체크포인트를 바라보는 방향)
	FTransform GetRespawnTransform() const;

	FORCEINLINE FName GetCheckpointRowName() const { return CheckpointRowName; }

protected:
	UPROPERTY(EditAnywhere, Category = "ET", meta = (GetOptions = "ET.ETGameDataSubsystem.GetCheckpointRowNameOptions"))
	FName CheckpointRowName;

	// 캡슐 중심 기준이므로 바닥에서 캡슐 절반 높이만큼 위에 배치
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USceneComponent> RespawnPointComponent;
};
