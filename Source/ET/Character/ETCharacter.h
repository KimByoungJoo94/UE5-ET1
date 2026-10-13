#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ETCharacter.generated.h"

class UETCharacterStateComponent;
class UETGameStatComponent;
class UETAttackCollisionComponent;
class UETTimeFreezeComponent;
class UETCharacterActionDataAsset;
class UAnimMontage;
struct FETCharacterActionMontageData;
struct FGameplayTag;

// PlayAction 으로 재생한 몽타주의 BlendingOut 시점 (끊긴 경우 포함)
DECLARE_DELEGATE_OneParam(FOnActionEnded, bool bInterrupted);

UCLASS()
class ET_API AETCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AETCharacter();

protected:
	virtual void PreInitializeComponents() override;

public:
	FORCEINLINE UETCharacterStateComponent* GetCharacterStateComponent() { return CharacterStateComponent; }
	FORCEINLINE UETGameStatComponent* GetGameStatComponent() { return GameStatComponent; }
	FORCEINLINE UETAttackCollisionComponent* GetAttackCollisionComponent() { return AttackCollisionComponent; }
	FORCEINLINE UETTimeFreezeComponent* GetTimeFreezeComponent() { return TimeFreezeComponent; }
	FORCEINLINE FName GetCharacterRowName() const { return CharacterRowName; }

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	// 실제로 들어간 데미지 반환 (대상이 회피 등으로 데미지를 받지 않으면 0)
	virtual float AttackTarget(AActor* InTarget, const FHitResult& InHitResult);
	virtual float GetAttackDamage() const;

	// 공격 시 바라볼 대상 (플레이어는 락온 대상, AI 는 블랙보드 대상)
	virtual AActor* GetAttackTarget() const { return nullptr; }
	virtual void ResetGameStat();
	void SetCapsuleHalfHeightKeepGround(const float InHalfHeight);
	void ResetCapsuleHalfHeight();
	float GetDefaultCapsuleHalfHeight() const;

	// 액션 데이터의 허용, 차단 상태 기준으로 현재 상태에서 행동 가능한지 (데이터에 없는 행동은 제한 없음)
	bool CanPlayAction(const FGameplayTag& InActionTag) const;

	// 액션 태그에 맞는 재생 가능한 몽타주가 있으면 상태 전환 후 재생 (AI 등 외부 호출용, InIndex 가 INDEX_NONE 이면 랜덤)
	bool PlayAction(const FGameplayTag& InActionTag, const int32 InIndex, const FGameplayTag& InStateTag, const FOnActionEnded& InOnActionEnded = FOnActionEnded());

	// 잠깐 이 캐릭터만 느리게 (타격감 연출, 시간 정지 중이면 무시)
	void StartHitStop(const float InDuration, const float InTimeDilation);

protected:
	// true 이면 데미지 무시 (회피, 패리 등 / 퍼펙트 판정 처리 위치)
	virtual bool TryAvoidDamage(const FDamageEvent& InDamageEvent, AActor* InDamageCauser);
	virtual void HitReact(AActor* InDamageCauser) {}
	// 사망 공통 처리 (판정, 이동, 충돌 정지 후 사망 몽타주 재생)
	virtual void Die();

	bool HasEnoughActionCost(const FETCharacterActionMontageData* InMontageData) const;
	void ConsumeActionCost(const FETCharacterActionMontageData* InMontageData);
		
	const FETCharacterActionMontageData* FindPlayableActionMontageData(const FGameplayTag& InActionTag, const int32 InIndex) const;
	
	// 찾은 데이터로 상태 전환 → 재생 → 비용 차감, BlendingOut 시 상태 복구 (찾기와 재생 사이에 처리가 필요한 경우 직접 사용)
	bool PlayAction(const FETCharacterActionMontageData* InMontageData, const FGameplayTag& InStateTag, const FOnActionEnded& InOnActionEnded = FOnActionEnded());

	// PlayAction 으로 재생한 몽타주의 BlendingOut 시점 (끊긴 경우 포함, 상태 복구 전 호출)
	virtual void OnActionMontageBlendingOut(const FGameplayTag& InStateTag, const bool bInterrupted) {}

private:
	void HandleActionMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted, FGameplayTag InStateTag, uint32 InActionSerial, FOnActionEnded InOnActionEnded);
	void EndHitStop();

protected:
	UPROPERTY(VisibleAnywhere, Category = "ET|Component")
	TObjectPtr<UETCharacterStateComponent> CharacterStateComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET|Component")
	TObjectPtr<UETGameStatComponent> GameStatComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET|Component")
	TObjectPtr<UETAttackCollisionComponent> AttackCollisionComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET|Component")
	TObjectPtr<UETTimeFreezeComponent> TimeFreezeComponent;

	UPROPERTY(EditDefaultsOnly, Category = "ET|Data", meta = (GetOptions = "ET.ETGameDataSubsystem.GetCharacterRowNameOptions"))
	FName CharacterRowName;

	UPROPERTY(Transient)
	TObjectPtr<UETCharacterActionDataAsset> CachedActionDataAsset;

private:
	// PlayAction 호출마다 증가, 이전 행동 몽타주의 늦은 BlendingOut 이 새 행동 상태를 덮어쓰지 않도록 구분
	uint32 ActionSerial = 0;

	FTimerHandle HitStopTimerHandle;
};
