#include "Character/ETCharacter.h"
#include "Components/ETCharacterStateComponent.h"
#include "Components/ETGameStatComponent.h"
#include "Components/ETWeaponCollisionComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

AETCharacter::AETCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	CharacterStateComponent = CreateDefaultSubobject<UETCharacterStateComponent>(TEXT("CharacterStateComponent"));
	GameStatComponent = CreateDefaultSubobject<UETGameStatComponent>(TEXT("GameStatComponent"));
	WeaponCollisionComponent = CreateDefaultSubobject<UETWeaponCollisionComponent>(TEXT("WeaponCollisionComponent"));
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

float AETCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Death))
	{
		return 0.f;
	}

	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.f)
	{
		return ActualDamage;
	}

	GameStatComponent->AddDepletedValue(EETGameStatType::Health, -ActualDamage);

	if (GameStatComponent->GetGameStat(EETGameStatType::Health).GetCurrentValue() <= 0.f)
	{
		CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Death);
		Die();
	}
	else
	{
		HitReact(DamageCauser);
	}

	return ActualDamage;
}

void AETCharacter::AttackTarget(AActor* InTarget, const FHitResult& InHitResult)
{
	UGameplayStatics::ApplyDamage(InTarget, GetAttackDamage(), GetController(), this, UDamageType::StaticClass());
}

float AETCharacter::GetAttackDamage() const
{
	return GameStatComponent->GetGameStat(EETGameStatType::Attack).GetCurrentValue();
}
