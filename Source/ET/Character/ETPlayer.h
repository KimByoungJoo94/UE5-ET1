#pragma once

#include "CoreMinimal.h"
#include "Character/ETCharacter.h"
#include "Interface/ETInteractionInterface.h"
#include "Interface/ETDefeatInterface.h"
#include "ETPlayer.generated.h"

class USpringArmComponent;
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
	TObjectPtr<UInputAction> ParryAction;

	UPROPERTY(EditDefaultsOnly, meta = (EditFixedSize))
	TArray<TObjectPtr<UInputAction>> ActiveSkillActionArray;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> UseItemAction;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UInputAction> LockOnAction;
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
	void OnActiveSkillActionStarted(const int32 InSkillIndex);
	void OnLockOnActionStarted();
	bool TryPayActiveSkillCost(const int32 InSkillIndex);
	void PlayActiveSkill(const int32 InSkillIndex);
	// ~Key Input

	void UseItem();
	void AdvanceComboAttack();
	void ResetComboAttack();
	void PauseHeavyAttack();
	void ResetHeavyAttack();
	void PauseParry();
	void ReleaseParry();
	void ResetParry();
	void SetPerfectTiming(const bool bInPerfectTiming);

	virtual float AttackTarget(AActor* InTarget, const FHitResult& InHitResult) override;
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
	virtual void HitReact(AActor* InDamageCauser) override;
	virtual void Die() override;

	virtual void OnActionMontageBlendingOut(const FGameplayTag& InStateTag, const bool bInterrupted) override;

	void PlayComboAttack();
	void PlayPerfectDodge();
	void PlayParryParticle(const FDamageEvent& InDamageEvent, const bool bInPerfect);
	void PlayCameraShake(const FName InCameraShakeRowName);
	void GainPerfectMomentum();

	void OnChargeCountChanged(const int32 InCurrentChargeCount, const int32 InMaxChargeCount);
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

	// 쓰러짐(ETAnimNotify_Defeat) 후 레벨 재시작까지 대기 시간
	UPROPERTY(EditDefaultsOnly, Category = "ET|Defeat", meta = (ClampMin = "0.01"))
	float DefeatRestartDelay = 3.f;

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

	// 회피, 패리 몽타주의 ETAnimNotifyState_PerfectTiming 구간 (이 구간에 공격이 닿으면 퍼펙트)
	bool bPerfectTiming = false;

private:
	FDelegateHandle OnChargeCountChangedHandle;};
