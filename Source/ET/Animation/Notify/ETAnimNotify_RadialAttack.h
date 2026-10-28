#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ETAnimNotify_RadialAttack.generated.h"

// 노티파이 시점에 오너(소켓 설정 시 소켓) 기준 원형 범위 안의 모든 캐릭터 공격
UCLASS()
class ET_API UETAnimNotify_RadialAttack : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetRadialAttackCollisionRowNameOptions"))
	FName RadialAttackCollisionRowName;

	// 유효하면 소켓 위치, 회전 기준으로 판정 (테이블 LocationOffset 도 소켓 로컬 기준), 비우면 오너 기준
	UPROPERTY(EditAnywhere)
	FName SocketName;
};
