#pragma once

#include "CoreMinimal.h"
#include "Character/ETCharacter.h"
#include "Interface/ETInteractionInterface.h"
#include "ETPlayer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UETInteractionComponent;
class UETChargeAttackComponent;
class UETAfterImageComponent;
class UETLockOnComponent;
class UInputMappingContext;
class UInputAction;
class UETCharacterActionDataAsset;
struct FETCharacterActionMontageData;
class UAnimMontage;
struct FInputActionValue;

USTRUCT()
struct FETPlayerInputData
{
	GENERATED_BODY()

public:
	FETPlayerInputData()
	{		
		ActiveSkillActionArray.Init(nullptr, 3);
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
struct FETPlayerTimeFreezeSkillData
{
	GENERATED_BODY()

public:
	// 플레이어 기준 탐색 반경
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0"))
	float SearchRadius = 1500.f;

	// 카메라 정면 기준 시야 반각
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0.0", ClampMax = "180.0"))
	float ViewHalfAngle = 45.f;

	// 가까운 순으로 적용할 최대 대상 수 (0 이면 범위 안 전체)
	UPROPERTY(EditDefaultsOnly, meta = (ClampMin = "0"))
	int32 MaxTargetCount = 1;

	// 0 이하이면 Unfreeze 호출 전까지 정지 유지
	UPROPERTY(EditDefaultsOnly)
	float FreezeDuration = 3.f;
};


UCLASS()
class ET_API AETPlayer : public AETCharacter
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
	void OnDodgeActionStarted();
	void OnParryActionStarted();
	void OnParryActionCompleted();
	void OnActiveSkillActionStarted(const int32 InSkillIndex);
	void UseTimeFreezeSkill(const int32 InSkillIndex);
	void GatherTimeFreezeTargets(OUT TArray<AETCharacter*>& OutTargetArray) const;
	void ApplyTimeFreeze(const TArray<AETCharacter*>& InTargetArray);
	bool TryPayActiveSkillCost(const int32 InSkillIndex);
	void PlayActiveSkill(const int32 InSkillIndex);
	// ~Key Input

	void AdvanceComboAttack();
	void ResetComboAttack();
	void PauseHeavyAttack();
	void ResetHeavyAttack();
	void PauseParry();
	void ReleaseParry();
	void ResetParry();

	virtual float GetAttackDamage() const override;

	UETInteractionComponent* GetInteractionComponent() { return InteractionComponent; }
	UETChargeAttackComponent* GetChargeAttackComponent() { return ChargeAttackComponent; }
	UETAfterImageComponent* GetAfterImageComponent() { return AfterImageComponent; }
	UETLockOnComponent* GetLockOnComponent() { return LockOnComponent; }
	const FText& GetPrimaryActionKeyText() const;

protected:
	virtual bool TryAvoidDamage(AActor* InDamageCauser) override;
	virtual void HitReact(AActor* InDamageCauser) override;
	virtual void Die() override;

	bool CanMove();
	bool CanPlayComboAttack();
	bool CanHeavyAttack();
	bool CanDodge();
	bool CanParry();
	bool CanUseActiveSkill();
	bool CanInteract();
	bool HasEnoughActionCost(const FETCharacterActionMontageData& InMontageData) const;
	void ConsumeActionCost(const FETCharacterActionMontageData& InMontageData);
void PlayComboAttack();

	void OnDodgeMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
	void PlayPerfectDodge();

	void OnParryMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
	void OnActiveSkillMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
	void OnInteractionMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);

	void OnChargeCountChanged(const int32 InCurrentChargeCount, const int32 InMaxChargeCount);
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UETInteractionComponent> InteractionComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UETChargeAttackComponent> ChargeAttackComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UETAfterImageComponent> AfterImageComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UETLockOnComponent> LockOnComponent;

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FETPlayerInputData PlayerInputData;

	UPROPERTY(EditDefaultsOnly, Category = "DataAsset")
	TObjectPtr<UETCharacterActionDataAsset> ActionDataAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge")
	FETPlayerDodgeData PlayerDodgeData;

	UPROPERTY(EditDefaultsOnly, Category = "Skill")
	FETPlayerTimeFreezeSkillData TimeFreezeSkillData;

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

private:
	FDelegateHandle OnChargeCountChangedHandle;
};
