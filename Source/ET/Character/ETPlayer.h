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
	void OnUseItemActionStarted();
	void OnDodgeActionStarted();
	void OnParryActionStarted();
	void OnParryActionCompleted();
	void OnActiveSkillActionStarted(const int32 InSkillIndex);
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
	bool CanUseItem();
	bool HasEnoughActionCost(const FETCharacterActionMontageData& InMontageData) const;
	void ConsumeActionCost(const FETCharacterActionMontageData& InMontageData);

	// 몽타주가 있고 비용이 충분한 행동 데이터 (없으면 nullptr)
	const FETCharacterActionMontageData* FindPlayableActionMontageData(const FGameplayTag& InActionTag, const int32 InIndex) const;
	// 상태 변경 후 재생, 성공 시 비용 차감 및 BlendingOut 바인딩 (실패 시 Idle 복구)
	bool PlayActionMontage(const FETCharacterActionMontageData& InMontageData, const FGameplayTag& InStateTag, void (ThisClass::*InBlendingOutFunc)(UAnimMontage*, bool));
	void PlayComboAttack();

	void OnDodgeMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
	void PlayPerfectDodge();

	void OnParryMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
	void OnActiveSkillMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
	void OnInteractionMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
	void OnUseItemMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);

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
