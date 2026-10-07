#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ETCharacter.generated.h"

class UETCharacterStateComponent;
class UETGameStatComponent;
class UETWeaponCollisionComponent;

UCLASS()
class ET_API AETCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AETCharacter();

protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;

	virtual void AttackTarget(AActor* InTarget, const FHitResult& InHitResult);
	virtual float GetAttackDamage() const;

	UETCharacterStateComponent* GetCharacterStateComponent() { return CharacterStateComponent; }
	UETGameStatComponent* GetGameStatComponent() { return GameStatComponent; }
	UETWeaponCollisionComponent* GetWeaponCollisionComponent() { return WeaponCollisionComponent; }

	void SetCapsuleHalfHeightKeepGround(const float InHalfHeight);
	void ResetCapsuleHalfHeight();
	float GetDefaultCapsuleHalfHeight() const;

protected:
	virtual void HitReact(AActor* InDamageCauser) {}
	virtual void Die() {}

protected:
	UPROPERTY(VisibleAnywhere, Category = "ET")
	TObjectPtr<UETCharacterStateComponent> CharacterStateComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET")
	TObjectPtr<UETGameStatComponent> GameStatComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET")
	TObjectPtr<UETWeaponCollisionComponent> WeaponCollisionComponent;
};
