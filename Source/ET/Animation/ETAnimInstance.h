#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ETAnimInstance.generated.h"

class AETCharacter;
class UCharacterMovementComponent;

// 원본 Paragon ABP 이벤트 그래프와 동일한 로직 (애님 그래프에서 읽는 값만 BlueprintReadOnly 로 노출)
UCLASS()
class ET_API UETAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	UETAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(Transient)
	TObjectPtr<AETCharacter> OwnerCharacter;

	UPROPERTY(Transient)
	TObjectPtr<UCharacterMovementComponent> CharacterMovementComponent;

	UPROPERTY(BlueprintReadOnly, Category = "ET")
	float Speed = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "ET")
	bool bIsInAir = false;

	UPROPERTY(BlueprintReadOnly, Category = "ET")
	bool bIsAccelerating = false;

	// 에임 회전 - 액터 회전 (AimOffset)
	UPROPERTY(BlueprintReadOnly, Category = "ET")
	float Roll = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "ET")
	float Pitch = 0.f;

	UPROPERTY(BlueprintReadOnly, Category = "ET")
	float Yaw = 0.f;

	// 초당 회전량 기반 Lean
	UPROPERTY(BlueprintReadOnly, Category = "ET")
	float YawDelta = 0.f;

	// 몽타주의 FullBody 커브가 켜진 구간
	UPROPERTY(BlueprintReadOnly, Category = "ET")
	bool bIsFullbody = false;

	// TODO : 락온 옆걸음 블렌드 스페이스 추가 시 사용
	// Actor forward 기준 이동 방향 (-180 ~ 180)
	UPROPERTY(BlueprintReadOnly, Category = "ET")
	float Direction = 0.f;

	// 이동 방향으로 회전하지 않는 상태 (락온 옆걸음)
	UPROPERTY(BlueprintReadOnly, Category = "ET")
	bool bStrafe = false;

private:
	FRotator RotationLastTick = FRotator::ZeroRotator;
};
