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
struct FETActionConditionTableRowData;
struct FGameplayTag;
enum class EETCharacterType : uint8;

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
	// 생성 시 FinishSpawning 전에만 지정 (PreInitializeComponents 에서 행 데이터 적용)
	FORCEINLINE void SetCharacterRowName(const FName InCharacterRowName) { CharacterRowName = InCharacterRowName; }

	// 캐릭터 테이블 행의 타입 (행이 없으면 Monster)
	EETCharacterType GetCharacterType() const;

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	// 실제로 들어간 데미지 반환 (대상이 회피 등으로 데미지를 받지 않으면 0, 발사체는 발사 시점 데미지 전달)
	virtual float AttackTarget(AActor* InTarget, const FHitResult& InHitResult, const float InDamage);
	virtual float GetAttackDamage() const;

	// 보상 테이블 행 적용 (영구 스탯은 스탯 컴포넌트에 반영)
	virtual void ApplyReward(const FName InRewardRowName);

	// 스턴 포인트 차감, 0 이 되면 다음 틱에 스턴 (스턴, 사망 중이거나 이미 0 이면 무시 / 최대값 0 은 스턴 면역)
	void ApplyStunDamage(const float InStunDamage);

	// 공격 시 바라볼 대상 (플레이어는 락온 대상, AI 는 블랙보드 대상)
	virtual AActor* GetAttackTarget() const { return nullptr; }
	virtual void ResetGameStat();
	void SetCapsuleHalfHeightKeepGround(const float InHalfHeight);
	void ResetCapsuleHalfHeight();
	float GetDefaultCapsuleHalfHeight() const;

	// 액션 데이터의 허용, 차단 상태 기준으로 현재 상태에서 행동 가능한지 (데이터에 없는 행동은 제한 없음, 인트로 중에는 사망 외 불가)
	bool CanPlayAction(const FGameplayTag& InActionTag) const;

	// 등장 연출 몽타주 재생 (Intro 액션 데이터가 없으면 재생하지 않고 false)
	virtual bool PlayIntro();

	// 액션 태그에 맞는 재생 가능한 몽타주가 있으면 상태 전환 후 재생 (AI 등 외부 호출용, InIndex 가 INDEX_NONE 이면 랜덤)
	bool PlayAction(const FGameplayTag& InActionTag, const int32 InIndex, const FGameplayTag& InStateTag, const FOnActionEnded& InOnActionEnded = FOnActionEnded());

	int32 GetActionMontageNum(const FGameplayTag& InActionTag) const;
	// 몽타주가 있고 액션 조건(비용, 쿨타임)을 만족하는 몽타주 데이터 (없으면 nullptr)
	const FETCharacterActionMontageData* FindPlayableActionMontageData(const FGameplayTag& InActionTag, const int32 InIndex) const;
	const FETActionConditionTableRowData* GetActionConditionRowData(const FETCharacterActionMontageData* InMontageData) const;

	// PlayAction 으로 재생 중인 행동의 몽타주 항목, 액션 조건 (재생 중인 행동이 없거나 조건 행이 없으면 nullptr)
	FORCEINLINE const FETCharacterActionMontageData* GetCurrentActionMontageData() const { return CurrentActionMontageData; }
	const FETActionConditionTableRowData* GetCurrentActionConditionRowData() const;

protected:
	// true 이면 데미지 무시 (회피, 패리 등 / 퍼펙트 판정 처리 위치)
	virtual bool TryAvoidDamage(const FDamageEvent& InDamageEvent, AActor* InDamageCauser);
	// Hit 액션 데이터의 허용, 차단 상태로 피격 몽타주 재생 여부 결정 (막혀도 데미지는 이미 적용됨)
	void HitReact(AActor* InDamageCauser);
	// 피격 몽타주 재생 직전 호출 (진행 중이던 행동 정리용)
	virtual void OnHitReact() {}
	// 처치한 캐릭터에게 보상 지급, 처치 기록 (보스 또는 1회 보상이 있는 캐릭터)
	void ProcessDefeat(AActor* InKiller);
	// 사망 공통 처리 (판정, 이동, 충돌 정지 후 사망 몽타주 재생)
	virtual void Die();
	// 스턴 몽타주 재생 (재생 실패 시 바로 종료 처리)
	virtual void Stun();
	// 스턴 몽타주 종료 시 스턴 포인트 초기화 (사망으로 끊긴 경우 제외)
	virtual void OnStunEnded(bool bInterrupted);
	// 인트로 몽타주 종료 (끊긴 경우 포함)
	virtual void OnIntroEnded(bool bInterrupted) {}

	// 몽타주 항목의 액션 조건 행 기준 (비용, 쿨타임), 행이 없으면 조건 없음
	bool IsActionConditionMet(const FETCharacterActionMontageData* InMontageData) const;
	// 비용 차감, 쿨타임 시작
	void ApplyActionCondition(const FETCharacterActionMontageData* InMontageData);
		
	// 찾은 데이터로 상태 전환 → 재생 → 비용 차감, BlendingOut 시 상태 복구 (찾기와 재생 사이에 처리가 필요한 경우 직접 사용)
	bool PlayAction(const FETCharacterActionMontageData* InMontageData, const FGameplayTag& InStateTag, const FOnActionEnded& InOnActionEnded = FOnActionEnded());

	// PlayAction 으로 재생한 몽타주의 BlendingOut 시점 (끊긴 경우 포함, 상태 복구 전 호출)
	virtual void OnActionMontageBlendingOut(const FGameplayTag& InStateTag, const bool bInterrupted) {}

private:
	// Character.Action.Hit 몽타주 배열 인덱스 (0 : Front, 1 : Back, 2 : Left, 3 : Right)
	int32 GetHitMontageIndex(const AActor* InDamageCauser) const;

	void HandleActionMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted, FGameplayTag InStateTag, uint32 InActionSerial, FOnActionEnded InOnActionEnded);

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
	// 액션 조건 RowName → 다시 사용 가능한 월드 시간
	TMap<FName, double> ActionCooldownEndTimeMap;

	// PlayAction 호출마다 증가, 이전 행동 몽타주의 늦은 BlendingOut 이 새 행동 상태를 덮어쓰지 않도록 구분
	uint32 ActionSerial = 0;

	// PlayAction 시점에 기록, 그 행동의 BlendingOut 때 해제 (액션 데이터 에셋 항목이라 에셋이 유지되는 동안 유효)
	const FETCharacterActionMontageData* CurrentActionMontageData = nullptr;
};
