#include "Character/ETCharacter.h"
#include "Components/ETCharacterStateComponent.h"
#include "Components/ETGameStatComponent.h"
#include "Components/ETAttackCollisionComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/DamageType.h"
#include "Kismet/GameplayStatics.h"

AETCharacter::AETCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	CharacterStateComponent = CreateDefaultSubobject<UETCharacterStateComponent>(TEXT("CharacterStateComponent"));
	GameStatComponent = CreateDefaultSubobject<UETGameStatComponent>(TEXT("GameStatComponent"));
	// BP 에 저장된 컴포넌트 설정 유지를 위해 서브오브젝트 이름은 기존 이름 유지
	AttackCollisionComponent = CreateDefaultSubobject<UETAttackCollisionComponent>(TEXT("WeaponCollisionComponent"));
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
	if (TryAvoidDamage(DamageCauser))
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

bool AETCharacter::TryAvoidDamage(AActor* InDamageCauser)
{
	return CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Death);
}

void AETCharacter::AttackTarget(AActor* InTarget, const FHitResult& InHitResult)
{
	UGameplayStatics::ApplyDamage(InTarget, GetAttackDamage(), GetController(), this, UDamageType::StaticClass());
}

float AETCharacter::GetAttackDamage() const
{
	const float Attack = GameStatComponent->GetGameStat(EETGameStatType::Attack).GetCurrentValue();

	// 기세 최대일 때 MomentumAttackBonus 비율만큼 공격력 증가 (0.5 = +50%)
	const FETGameStat& MomentumStat = GameStatComponent->GetGameStat(EETGameStatType::Momentum);
	const float MomentumRatio = MomentumStat.GetMaxValue() > 0.f ? MomentumStat.GetCurrentValue() / MomentumStat.GetMaxValue() : 0.f;
	const float MomentumAttackBonus = GameStatComponent->GetGameStat(EETGameStatType::MomentumAttackBonus).GetCurrentValue();

	return Attack * (1.f + MomentumRatio * MomentumAttackBonus);
}

void AETCharacter::SetCapsuleHalfHeightKeepGround(const float InHalfHeight)
{
	UCapsuleComponent* Capsule = GetCapsuleComponent();
	const float OldScaledHalfHeight = Capsule->GetScaledCapsuleHalfHeight();

	Capsule->SetCapsuleHalfHeight(InHalfHeight);

	// 캡슐 바닥을 지면에 유지하도록 변경된 높이만큼 이동 (메시도 함께 이동)
	const float HeightOffset = Capsule->GetScaledCapsuleHalfHeight() - OldScaledHalfHeight;
	if (FMath::IsNearlyZero(HeightOffset) == false)
	{
		AddActorWorldOffset(FVector(0.f, 0.f, HeightOffset));

		// 카메라 높이가 캡슐 높이 변화를 따라가지 않도록 반대로 보정
		if (USpringArmComponent* SpringArmComponent = FindComponentByClass<USpringArmComponent>())
		{
			SpringArmComponent->AddRelativeLocation(FVector(0.f, 0.f, -HeightOffset));
		}
	}
}

void AETCharacter::ResetCapsuleHalfHeight()
{
	SetCapsuleHalfHeightKeepGround(GetDefaultCapsuleHalfHeight());
}

float AETCharacter::GetDefaultCapsuleHalfHeight() const
{
	const ACharacter* DefaultCharacter = GetClass()->GetDefaultObject<ACharacter>();
	return DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight();
}
