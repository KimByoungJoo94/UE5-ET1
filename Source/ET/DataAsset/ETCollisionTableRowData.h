#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ETCollisionTableRowData.generated.h"

class UParticleSystem;

// 무기 소켓 사이 스윕 판정 (노티파이 스테이트 구간 동안 매 틱)
USTRUCT()
struct ET_API FETWeaponCollisionTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	FETWeaponCollisionTableRowData();

	UPROPERTY(EditAnywhere)
	FName StartSocketName;

	UPROPERTY(EditAnywhere)
	FName EndSocketName;

	// 소켓(본) 로컬 공간 기준 위치 보정, 소켓 회전을 따라감 (무기 끝 소켓이 없으면 Start, End 를 같은 소켓으로 두고 이 값으로 양 끝 지정)
	UPROPERTY(EditAnywhere)
	FVector StartLocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	FVector EndLocationOffset = FVector::ZeroVector;

	// Start → End 소켓 위치를 잇는 무기 축 방향 길이 보정 (+ 늘림, - 줄임, 소켓 회전은 사용하지 않음)
	UPROPERTY(EditAnywhere)
	float StartLengthOffset = 0.f;

	UPROPERTY(EditAnywhere)
	float EndLengthOffset = 0.f;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float Radius = 20.f;

	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeArray;

	// 실제 데미지가 들어갔을 때 충돌 지점에 출력
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UParticleSystem> HitParticle;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float HitParticleScale = 1.f;

	// 실제 데미지가 들어갔을 때 카메라 쉐이크 (때린 쪽, 맞은 쪽 상관없이 로컬 플레이어 카메라)
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetCameraShakeRowNameOptions"))
	FName CameraShakeRowName;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::None;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float DrawDebugTime = 2.f;
};

// 오너 기준 원형 범위 공격 (노티파이 시점 1회 판정)
USTRUCT()
struct ET_API FETRadialAttackCollisionTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	FETRadialAttackCollisionTableRowData();

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float Radius = 300.f;

	// 오너 로컬 기준 오프셋 (X 전방, Y 우측, Z 위), 노티파이에 소켓 설정 시 소켓 로컬 기준
	UPROPERTY(EditAnywhere)
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeArray;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UParticleSystem> CenterParticle;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float CenterParticleScale = 1.f;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UParticleSystem> HitParticle;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float HitParticleScale = 1.f;

	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetCameraShakeRowNameOptions"))
	FName CameraShakeRowName;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::None;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float DrawDebugTime = 2.f;
};
