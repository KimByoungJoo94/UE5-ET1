#pragma once

#include "CoreMinimal.h"
#include "Character/ETCharacter.h"
#include "Interface/ETCombatInterface.h"
#include "Interface/ETInteractionInterface.h"
#include "ETPlayer.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UETInteractionComponent;
class UInputMappingContext;
class UInputAction;
class UETCharacterActionDataAsset;
class UAnimMontage;
struct FInputActionValue;

USTRUCT()
struct FETPlayerInputData
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputMappingContext> InputMappingContext;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> AttackAction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> HeavyAttackAction;

	UPROPERTY(EditAnywhere)
	TObjectPtr<UInputAction> PrimaryAction;
};

UCLASS()
class ET_API AETPlayer : public AETCharacter, public IETCombatInterface
{
	GENERATED_BODY()
	
public:
	AETPlayer();

protected:
	virtual void BeginPlay() override;
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
	// ~Key Input

	void AdvanceComboAttack();
	void ResetComboAttack();
	void ResetHeavyAttack();

	// IETCombatInterface
	virtual void OnStartWeaponCollision() override;
	virtual void OnEndWeaponCollision() override;
	virtual void OnHeavyAttackPause() override;	
	virtual void OnHit() override;
	virtual void OnDeath() override;
	// ~IETCombatInterface

	UETInteractionComponent* GetInteractionComponent() { return InteractionComponent; }
	const FText& GetPrimaryActionKeyText() const;

protected:
	bool CanMove();
	bool CanPlayComboAttack();
	bool CanHeavyAttack();
	void PlayComboAttack();
	
protected:
	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<USpringArmComponent> SpringArmComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere, Category = "Component")
	TObjectPtr<UETInteractionComponent> InteractionComponent;

protected:
	UPROPERTY(EditAnywhere, Category = "Input")
	FETPlayerInputData PlayerInputData;

	UPROPERTY(EditAnywhere, Category = "DataAsset")
	TObjectPtr<UETCharacterActionDataAsset> ActionDataAsset;

protected:
	bool bComboAttackReserved = false;	
	int32 ComboAttackIndex = 0;

	bool bPlayHeavyAttack = false;
		
	UPROPERTY(Transient)
	TObjectPtr<UAnimMontage> HeavyAttackMontage;
};
