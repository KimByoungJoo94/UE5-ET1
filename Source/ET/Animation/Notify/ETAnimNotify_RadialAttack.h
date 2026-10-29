#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ETAnimNotify_RadialAttack.generated.h"

// 노티파이 시점에 오너(소켓 설정 시 소켓) 위치에 오프셋을 더한 중심의 원형 범위 안 모든 캐릭터 공격 (범위, 충돌은 테이블)
UCLASS()
class ET_API UETAnimNotify_RadialAttack : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetRadialAttackCollisionRowNameOptions"))
	FName RadialAttackCollisionRowName;

	// 유효하면 소켓 위치, 회전 기준으로 판정, 비우면 오너 기준
	UPROPERTY(EditAnywhere)
	FName SocketName;

	// 소켓(없으면 오너) 로컬 기준 오프셋 (X 전방, Y 우측, Z 위)
	UPROPERTY(EditAnywhere)
	FVector LocationOffset = FVector::ZeroVector;

	// 중심 파티클 회전 오프셋 (소켓(없으면 오너) 회전 기준, 엔진 Play Particle Effect 와 동일)
	UPROPERTY(EditAnywhere)
	FRotator RotationOffset = FRotator::ZeroRotator;
};
