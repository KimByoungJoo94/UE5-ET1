#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ETLockOnComponent.generated.h"

class AETCharacter;

// 휠 버튼 락온 (대상 고정 카메라, 락온 중 옆걸음)
UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETLockOnComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UETLockOnComponent();

	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

public:
	void ToggleLockOn();
	void StartLockOn(AETCharacter* InTarget);
	void EndLockOn();

	FORCEINLINE bool IsLockedOn() const { return LockOnTarget.IsValid(); }
	AETCharacter* GetLockOnTarget() const;

protected:
	// IETLockOnInterface 구현 캐릭터 중 탐색 반경, 카메라 시야각, 시야 차단으로 거른 뒤 화면 중앙 각도와 거리 가중합이 가장 낮은 캐릭터
	AETCharacter* FindLockOnTarget() const;

	bool IsTargetDead(AETCharacter* InTarget) const;
	FVector GetTargetLookAtLocation(const AETCharacter* InTarget) const;

	void UpdateControlRotation(const float InDeltaTime);
	void UpdateOwnerRotationMode();
	void SetStrafeMode(const bool bInStrafe);

protected:
	UPROPERTY(EditAnywhere, Category = "LockOn", meta = (ClampMin = "0.0"))
	float SearchRadius = 1500.f;

	// 카메라 정면 기준 반각
	UPROPERTY(EditAnywhere, Category = "LockOn", meta = (ClampMin = "1.0", ClampMax = "180.0"))
	float ViewHalfAngle = 60.f;

	// 1 이면 각도만, 0 이면 거리만 사용
	UPROPERTY(EditAnywhere, Category = "LockOn", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AngleWeight = 0.7f;

	// 대상 액터 위치(캡슐 중심) 기준 바라볼 높이
	UPROPERTY(EditAnywhere, Category = "LockOn")
	float TargetHeightOffset = 50.f;

	// 이 높이(Owner 위치 기준)에서 대상을 바라보는 회전으로 카메라를 설정
	// 대상이 가까울수록 더 내려다보게 되어 카메라가 위로 올라가고 플레이어에 가려지지 않음
	UPROPERTY(EditAnywhere, Category = "LockOn")
	float ViewPivotHeightOffset = 150.f;

	// 대상을 약간 내려다보도록 더하는 Pitch
	UPROPERTY(EditAnywhere, Category = "LockOn")
	float PitchOffset = -10.f;

	UPROPERTY(EditAnywhere, Category = "LockOn")
	float MinPitch = -45.f;

	UPROPERTY(EditAnywhere, Category = "LockOn")
	float MaxPitch = 15.f;

	UPROPERTY(EditAnywhere, Category = "LockOn", meta = (ClampMin = "0.0"))
	float RotationInterpSpeed = 10.f;

private:
	TWeakObjectPtr<AETCharacter> LockOnTarget;
};
