#pragma once

#include "CoreMinimal.h"
#include "Character/ETCharacter.h"
#include "Interface/ETInteractionInterface.h"
#include "Interface/ETDefeatInterface.h"
#include "ETPlayer.generated.h"

enum class EETGameStatType : uint8;

class USpringArmComponent;
class AETWaterPlane;
class UCameraComponent;
class UETInteractionComponent;
class UETChargeAttackComponent;
class UETAfterImageComponent;
class UETLockOnComponent;
class UETDamageTextComponent;
class UInputMappingContext;
class UInputAction;
class UAnimMontage;
class UParticleSystem;
class UParticleSystemComponent;

struct FInputActionValue;
struct FGameplayTag;

USTRUCT()
struct FETPlayerInputData
{
	GENERATED_BODY()

public:
	FETPlayerInputData()
	{		
		ActiveSkillActionArray.Init(nullptr, 4);
	}

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> HeavyAttackAction;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> PrimaryAction;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> DodgeAction;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> SprintAction;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> ParryAction;

	UPROPERTY(EditDefaultsOnly, meta = (EditFixedSize))
	TArray<TObjectPtr<UInputAction>> ActiveSkillActionArray;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> UseItemAction;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> LockOnAction;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> MenuAction;
};

USTRUCT()
struct FETPlayerDodgeData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0"))
	float DodgeDistance = 450.f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.01"))
	float DodgeDuration = 0.6f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float PerfectDodgeTimeDilation = 0.3f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0"))
	float PerfectDodgeSlowDuration = 0.5f;
};

USTRUCT()
struct FETPlayerParryData
{
	GENERATED_BODY()

public:
	// 패리로 막은 충돌 지점에 출력
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UParticleSystem> ParryParticle;

	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UParticleSystem> PerfectParryParticle;

	// 패리 시 플레이어와 공격자를 잠깐 느리게
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0"))
	float ParryHitStopDuration = 0.05f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0"))
	float PerfectParryHitStopDuration = 0.12f;

	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float HitStopTimeDilation = 0.05f;
};

USTRUCT()
struct FETPlayerChargeParticleData
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditDefaultsOnly)
	TSoftObjectPtr<UParticleSystem> Particle;

	UPROPERTY(EditDefaultsOnly)
	FName SocketName;
};

USTRUCT()
struct FETPlayerChargeAttackData
{
	GENERATED_BODY()

public:	
	UPROPERTY(EditDefaultsOnly)
	TArray<FETPlayerChargeParticleData> ChargeParticleArray;
};


UCLASS()
class ET_API AETPlayer : public AETCharacter, public IETDefeatInterface
{
	GENERATED_BODY()
	
public:
	AETPlayer();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
	virtual void NotifyControllerChanged() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void CalcCamera(float DeltaTime, struct FMinimalViewInfo& OutResult) override;
	virtual bool CanJumpInternal_Implementation() const override;
virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
		
public:
	// Key Input
	void OnMoveActionTriggered(const FInputActionValue& InValue);
	void OnLookActionTriggered(const FInputActionValue& InValue);
	void OnAttackActionCompleted();	
	void OnHeavyAttackActionStarted();
	void OnHeavyAttackActionCompleted();
	void OnPrimaryActionCompleted();
	void OnUseItemActionStarted();
	void OnDodgeActionStarted();
	void OnParryActionStarted();
	void OnParryActionCompleted();
	void OnSprintActionStarted();
	void OnSprintActionCompleted();
	void OnActiveSkillActionStarted(const int32 InSkillIndex);
	void OnLockOnActionStarted();
	void OnMenuActionStarted();
	bool TryPayActiveSkillCost(const int32 InSkillIndex);
	void PlayActiveSkill(const int32 InSkillIndex);
	// ~Key Input

	// 회복 플라스크 1회 사용 (최대 체력의 HealFlaskPercent % 회복, 남은 횟수가 없으면 무시)
	void UseItem();

	// 영구 스탯은 세이브 데이터에도 반영 (디스크 저장은 체크포인트 / 사망 / 게임 종료 시)
	virtual void ApplyReward(const FName InRewardRowName) override;
	void AdvanceComboAttack();
	void ResetComboAttack();
	void PauseHeavyAttack();
	void ResetHeavyAttack();
	void PauseParry();
	void ReleaseParry();
	void ResetParry();
	void SetPerfectTiming(const bool bInPerfectTiming);

	// 기믹 순간이동 (락온 해제, 상호작용 동작 정지, 카메라를 도착 방향으로)
	void TeleportByGimmick(const FTransform& InTransform);

	virtual float AttackTarget(AActor* InTarget, const FHitResult& InHitResult, const float InDamage) override;
	virtual float GetAttackDamage() const override;
	virtual AActor* GetAttackTarget() const override;

	UETInteractionComponent* GetInteractionComponent() { return InteractionComponent; }
	UETChargeAttackComponent* GetChargeAttackComponent() { return ChargeAttackComponent; }
	UETAfterImageComponent* GetAfterImageComponent() { return AfterImageComponent; }
	UETLockOnComponent* GetLockOnComponent() { return LockOnComponent; }
	const FText& GetPrimaryActionKeyText() const;

	// IETDefeatInterface
	virtual void OnDefeated() override;
	// ~IETDefeatInterface

protected:
	virtual bool TryAvoidDamage(const FDamageEvent& InDamageEvent, AActor* InDamageCauser) override;
	virtual void OnHitReact() override;
	virtual void Die() override;

	virtual void OnActionMontageBlendingOut(const FGameplayTag& InStateTag, const bool bInterrupted) override;

	void PlayComboAttack();
	void PlayPerfectDodge();
	void PlayParryParticle(const FDamageEvent& InDamageEvent, const bool bInPerfect);
	void GainPerfectMomentum();
	// 퍼펙트 회피, 패리 시 공격한 캐릭터의 스턴 포인트 차감
	void ApplyPerfectStunDamage(AActor* InDamageCauser);

	// 스폰 시 세이브 데이터의 영구 스탯을 스탯 컴포넌트에 적용
	void ApplySavedPermanentGameStat();

	void OnChargeCountChanged(const int32 InCurrentChargeCount, const int32 InMaxChargeCount);

	// 카메라가 수면 아래로 내려가지 않도록 높이만 수면 위로 올림 (거리, 방향은 유지해서 캐릭터 쪽으로 당겨지지 않음)
	void KeepCameraAboveWater(OUT FMinimalViewInfo& OutViewInfo) const;

protected:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET|Component")
	TObjectPtr<UETInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET|Component")
	TObjectPtr<UETChargeAttackComponent> ChargeAttackComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET|Component")
	TObjectPtr<UETAfterImageComponent> AfterImageComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET|Component")
	TObjectPtr<UETLockOnComponent> LockOnComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET|Component")
	TObjectPtr<UETDamageTextComponent> DamageTextComponent;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "ET|Input")
	FETPlayerInputData PlayerInputData;

	UPROPERTY(EditDefaultsOnly, Category = "ET|Dodge")
	FETPlayerDodgeData PlayerDodgeData;

	UPROPERTY(EditDefaultsOnly, Category = "ET|Parry")
	FETPlayerParryData PlayerParryData;

	UPROPERTY(EditDefaultsOnly, Category = "ET|ChargeAttack")
	FETPlayerChargeAttackData PlayerChargeAttackData;

protected:
	bool bComboAttackReserved = false;	
	int32 ComboAttackIndex = 0;

	bool bPlayHeavyAttack = false;
		
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

	bool bParryPaused = false;
	bool bParryReleased = false;

	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> ParryMontage;

	UPROPERTY(Transient)
	TArray<TObjectPtr<UParticleSystemComponent>> ChargeParticleSystemComponentArray;
		
	bool bPerfectTiming = false;

private:
	FDelegateHandle OnChargeCountChangedHandle;

	// 레벨의 수면 (없으면 카메라 보정 없음, 레벨당 하나라 시작 시 한 번 찾음)
	TWeakObjectPtr<AETWaterPlane> WaterPlane;
};
