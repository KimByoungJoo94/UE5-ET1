#include "Character/ETCharacter.h"
#include "Components/ETCharacterStateComponent.h"
#include "Components/ETGameStatComponent.h"
#include "Components/ETCombatComponent.h"

AETCharacter::AETCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	CharacterStateComponent = CreateDefaultSubobject<UETCharacterStateComponent>(TEXT("CharacterStateComponent"));
	GameStatComponent = CreateDefaultSubobject<UETGameStatComponent>(TEXT("GameStatComponent"));
	CombatComponent = CreateDefaultSubobject<UETCombatComponent>(TEXT("CombatComponent"));
}

void AETCharacter::BeginPlay()
{
	Super::BeginPlay();
}

void AETCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AETCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
}

