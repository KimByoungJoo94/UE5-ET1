#pragma once

#include "CoreMinimal.h"
#include "Character/ETCharacter.h"
#include "Interface/ETInteractionInterface.h"
#include "ETPlayer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UETInteractionComponent;
class UETChargeAttackComponent;
class UInputMappingContext;
class UInputAction;
class UETCharacterActionDataAsset;
class UAnimMontage;
class UMaterialInterface;
struct FInputActionValue;

USTRUCT()
struct FETPlayerInputData
{
	GENERATED_BODY()

public:
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
	// ~Key Input

	void AdvanceComboAttack();
	void ResetComboAttack();
	void PauseHeavyAttack();
	void ResetHeavyAttack();

	virtual float GetAttackDamage() const override;

	UETInteractionComponent* GetInteractionComponent() { return InteractionComponent; }
	UETChargeAttackComponent* GetChargeAttackComponent() { return ChargeAttackComponent; }
	const FText& GetPrimaryActionKeyText() const;

protected:
	virtual void HitReact(AActor* InDamageCauser) override;
	virtual void Die() override;

	bool CanMove();
	bool CanPlayComboAttack();
	bool CanHeavyAttack();
	bool CanDodge();
	void PlayComboAttack();

	void OnDodgeMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted);
	void PlayPerfectDodge();

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

protected:
	UPROPERTY(EditDefaultsOnly, Category = "Input")
	FETPlayerInputData PlayerInputData;

	UPROPERTY(EditDefaultsOnly, Category = "DataAsset")
	TObjectPtr<UETCharacterActionDataAsset> ActionDataAsset;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (ClampMin = "0.0"))
	float DodgeDistance = 450.f;

	UPROPERTY(EditDefaultsOnly, Category = "Dodge", meta = (ClampMin = "0.01"))
	float DodgeDuration = 0.6f;

	UPROPERTY(EditDefaultsOnly, Category = "PerfectDodge")
	TObjectPtr<UMaterialInterface> AfterImageMaterial;

	UPROPERTY(EditDefaultsOnly, Category = "PerfectDodge", meta = (ClampMin = "0.01"))
	float AfterImageLifeTime = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "PerfectDodge", meta = (ClampMin = "0.0", ClampMax = "1.0"))
	float AfterImageOpacity = 0.5f;

	UPROPERTY(EditDefaultsOnly, Category = "PerfectDodge", meta = (ClampMin = "0.01", ClampMax = "1.0"))
	float PerfectDodgeTimeDilation = 0.3f;

	UPROPERTY(EditDefaultsOnly, Category = "PerfectDodge", meta = (ClampMin = "0.0"))
	float PerfectDodgeSlowDuration = 1.f;

protected:
	bool bComboAttackReserved = false;	
	int32 ComboAttackIndex = 0;

	bool bPlayHeavyAttack = false;
		
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> HeavyAttackMontage;

private:
	FDelegateHandle OnChargeCountChangedHandle;
};
