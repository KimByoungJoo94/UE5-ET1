#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimNotifies/AnimNotify.h"
#include "ETAnimNotify_FireProjectile.generated.h"

// 노티파이 시점에 Start → End 소켓 방향으로 발사체 생성 (총구 뒤 → 총구 앞, 발사 위치는 End)
UCLASS()
class ET_API UETAnimNotify_FireProjectile : public UAnimNotify
{
	GENERATED_BODY()

public:
	virtual void Notify(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference) override;

protected:
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetProjectileRowNameOptions"))
	FName ProjectileRowName;

	UPROPERTY(EditAnywhere)
	FName StartSocketName;

	UPROPERTY(EditAnywhere)
	FName EndSocketName;

	// 소켓(본) 로컬 공간 기준 위치 보정, 소켓 회전을 따라감 (소켓이 하나면 Start, End 를 같은 소켓으로 두고 이 값으로 양 끝 지정)
	UPROPERTY(EditAnywhere)
	FVector StartLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FVector EndLocationOffset = FVector::ZeroVector;
};
