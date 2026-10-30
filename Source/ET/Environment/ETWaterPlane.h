#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ETWaterPlane.generated.h"

class AETCharacter;
class UBoxComponent;
class UStaticMeshComponent;

// 수면 평면 (수면 높이는 메시 위치, 수면 아래 영역에서 캐릭터 몸 전체가 잠기면 익사, 플레이어 카메라는 수면 위로 보정)
UCLASS()
class ET_API AETWaterPlane : public AActor
{
	GENERATED_BODY()

public:
	AETWaterPlane();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void Tick(float DeltaSeconds) override;

public:
	float GetSurfaceHeight() const;
	FORCEINLINE float GetCameraSurfaceMargin() const { return CameraSurfaceMargin; }

	// 수면 영역의 수평 범위 안인지 (높이 무시)
	bool IsInWaterArea(const FVector& InLocation) const;

private:
	UFUNCTION()
	void OnWaterBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);

	UFUNCTION()
	void OnWaterBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

	// 수면 메시 바운드 수평 범위, 수면부터 WaterDepth 깊이까지 판정 영역 맞춤
	void UpdateWaterBox();

	// 다른 컴포넌트(무기 등) 오버랩은 무시하고 캐릭터 캡슐만
	AETCharacter* GetCharacterByCapsule(AActor* InOtherActor, const UPrimitiveComponent* InOtherComp) const;

protected:
	// 맵 전체를 덮는 경우가 많아 루트로 두고 배치한 Transform 그대로 사용 (기존 평면 액터를 대체해도 위치, 스케일 유지)
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UStaticMeshComponent> WaterMeshComponent;

	// 수면 아래 영역 (Pawn 오버랩)
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UBoxComponent> WaterBoxComponent;

	// 수면부터 판정 영역 바닥까지 깊이 (물 아래 지형보다 깊게)
	UPROPERTY(EditAnywhere, Category = "ET", meta = (ClampMin = "0.0"))
	float WaterDepth = 5000.f;

	// 플레이어 카메라가 유지할 수면 위 최소 높이
	UPROPERTY(EditAnywhere, Category = "ET", meta = (ClampMin = "0.0"))
	float CameraSurfaceMargin = 30.f;

private:
	// 판정 영역에 캡슐이 들어온 캐릭터 (있는 동안만 Tick 에서 몸 전체가 잠겼는지 확인)
	TSet<TWeakObjectPtr<AETCharacter>> InWaterCharacterSet;
};
