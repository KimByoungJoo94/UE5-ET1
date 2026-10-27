#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Engine/EngineTypes.h"
#include "ETProjectileTableRowData.generated.h"

class UParticleSystem;

UENUM()
enum class EETProjectileMoveType : uint8
{
	// 발사 방향으로 중력 없이 이동
	Straight = 0,
	// 발사 방향으로 쏘고 중력을 받아 떨어짐
	Parabolic,
};

// 발사체 구성 (발사 위치, 방향은 노티파이의 소켓 기준, 데미지는 발사 시점 발사자 공격력)
USTRUCT()
struct ET_API FETProjectileTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	FETProjectileTableRowData();

	UPROPERTY(EditAnywhere)
	EETProjectileMoveType MoveType = EETProjectileMoveType::Straight;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float Speed = 2000.f;

	// 월드 중력 배율 (1 = 기본 중력)
	UPROPERTY(EditAnywhere, meta = (EditCondition = "MoveType == EETProjectileMoveType::Parabolic", ClampMin = "0.0"))
	float GravityScale = 1.f;

	// 충돌 구 반지름
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float Radius = 20.f;

	// 맞히는 대상 (지형은 항상 막혀서 소멸)
	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeArray;

	// 아무것도 맞지 않으면 이 시간 뒤 소멸 (0 이면 무제한)
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float MaxLifeTime = 5.f;

	// 발사체 루트에 붙어 날아가는 동안 재생 (Ribbon 이미터가 있으면 꼬리가 남음)
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UParticleSystem> RootParticle;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float RootParticleScale = 1.f;

	// 지형에 맞거나 대상에게 실제 데미지가 들어갔을 때 충돌 지점에 출력
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UParticleSystem> HitParticle;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float HitParticleScale = 1.f;
};
