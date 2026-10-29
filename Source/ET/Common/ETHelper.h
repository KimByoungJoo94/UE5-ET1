#pragma once

#include "CoreMinimal.h"

class UParticleSystem;
class UParticleSystemComponent;
class USceneComponent;
enum class EETCharacterType : uint8;

// 특정 기능에 속하지 않는 공용 전역 함수 모음 (기능 색이 확실한 함수는 해당 클래스나 네임스페이스에 정의)
class ET_API FETHelper
{
public:
	// 위치에 파티클 출력 (루프 파티클은 끝나지 않으므로 일정 시간 뒤 정지)
	static void SpawnParticleAtLocation(const UObject* InWorldContextObject, const TSoftObjectPtr<UParticleSystem>& InParticle, const FVector& InLocation, const float InScale);

	// 컴포넌트 소켓에 붙여 파티클 출력 (사라질 때까지 따라다님, 루프 파티클은 반환한 컴포넌트로 호출한 쪽에서 정지)
	static UParticleSystemComponent* SpawnParticleAttached(const TSoftObjectPtr<UParticleSystem>& InParticle, USceneComponent* InAttachComponent, const FName InSocketName);

	// 플레이어와 몬스터(보스 포함)만 서로 적 (같은 편은 공격이 닿아도 데미지 없음)
	static bool IsHostile(const EETCharacterType InAttackerType, const EETCharacterType InTargetType);

	// 정면 기준 시야각 안에 있고 지형에 가려지지 않았는지 (다른 캐릭터에 가려지는 건 무시)
	static bool IsInSight(const AActor* InOwner, const AActor* InTarget, const float InSightAngle);
};
