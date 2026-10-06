#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "ETCharacter.generated.h"

class UETCharacterStateComponent;
class UETGameStatComponent;
class UETCombatComponent;

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
	
	UETCharacterStateComponent* GetCharacterStateComponent() { return CharacterStateComponent; }
	UETGameStatComponent* GetGameStatComponent() { return GameStatComponent; }
	UETCombatComponent* GetCombatComponent() { return CombatComponent; }

protected:
	UPROPERTY(VisibleAnywhere, Category = "ET")
	TObjectPtr<UETCharacterStateComponent> CharacterStateComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET")
	TObjectPtr<UETGameStatComponent> GameStatComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET")
	TObjectPtr<UETCombatComponent> CombatComponent;
};
