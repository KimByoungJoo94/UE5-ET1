#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "Kismet/KismetSystemLibrary.h"
#include "ETCollisionTableRowData.generated.h"

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

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float Radius = 20.f;

	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeArray;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::ForDuration;

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

	// 오너 로컬 기준 오프셋 (X 전방, Y 우측, Z 위)
	UPROPERTY(EditAnywhere)
	FVector LocationOffset = FVector::ZeroVector;

	UPROPERTY(EditAnywhere)
	TArray<TEnumAsByte<EObjectTypeQuery>> ObjectTypeArray;

	UPROPERTY(EditAnywhere)
	TEnumAsByte<EDrawDebugTrace::Type> DrawDebugType = EDrawDebugTrace::ForDuration;

	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.0"))
	float DrawDebugTime = 2.f;
};
