#include "Character/ETCharacter.h"
#include "Components/ETCharacterStateComponent.h"
#include "Components/ETGameStatComponent.h"
#include "Components/ETAttackCollisionComponent.h"
#include "Components/ETTimeFreezeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DataAsset/ETCharacterActionDataAsset.h"
#include "DataAsset/ETCharacterTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"

AETCharacter::AETCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	CharacterStateComponent = CreateDefaultSubobject<UETCharacterStateComponent>(TEXT("CharacterStateComponent"));
	GameStatComponent = CreateDefaultSubobject<UETGameStatComponent>(TEXT("GameStatComponent"));	
	AttackCollisionComponent = CreateDefaultSubobject<UETAttackCollisionComponent>(TEXT("AttackCollisionComponent"));
	TimeFreezeComponent = CreateDefaultSubobject<UETTimeFreezeComponent>(TEXT("TimeFreezeComponent"));

	// 경로 이동(AI MoveTo)도 가속도로 처리 (기본값이면 경로 이동 중 CurrentAcceleration 이 0 이라 애님이 이동으로 전환되지 않음)
	GetCharacterMovement()->GetNavMovementProperties()->bUseAccelerationForPaths = true;
}

void AETCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	const FETCharacterTableRowData* CharacterRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETCharacterTableRowData>(CharacterRowName);
	if (ensureMsgf(CharacterRowData, TEXT("Invalid CharacterRowName. Character: %s, RowName: %s"), *GetName(), *CharacterRowName.ToString()) == false)
	{
		return;
	}

	GameStatComponent->GenerateGameStat(CharacterRowData->BaseGameStatRowName);

	CachedActionDataAsset = CharacterRowData->ActionDataAsset.LoadSynchronous();
}

float AETCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (TryAvoidDamage(DamageEvent, DamageCauser))
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

bool AETCharacter::TryAvoidDamage(const FDamageEvent& InDamageEvent, AActor* InDamageCauser)
{
	return CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Death);
}

void AETCharacter::Die()
{
	AttackCollisionComponent->EndWeaponCollision();

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// 사망 후에는 맞지도 길을 막지도 않도록
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 상태 컴포넌트가 Death 에서 다른 상태로 바뀌지 않으므로 몽타주가 끝나도 Death 유지
	if (PlayAction(ETGameplayTags::Character_Action_Death, INDEX_NONE, ETGameplayTags::Character_State_Death) == false)
	{
		return;
	}

	// 에셋 설정과 상관없이 마지막 프레임(쓰러진 자세)에서 정지
	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		if (FAnimMontageInstance* MontageInstance = AnimInstance->GetActiveMontageInstance())
		{
			MontageInstance->bEnableAutoBlendOut = false;
		}
	}
}

float AETCharacter::AttackTarget(AActor* InTarget, const FHitResult& InHitResult)
{
	// 대상의 TakeDamage 반환값 (포인트 데미지로 충돌 정보를 대상에게 전달)
	const FVector HitFromDirection = (InTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	return UGameplayStatics::ApplyPointDamage(InTarget, GetAttackDamage(), HitFromDirection, InHitResult, GetController(), this, UDamageType::StaticClass());
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

void AETCharacter::ResetGameStat()
{
	GameStatComponent->ResetAllDepletedValue();

	// 기세는 0 에서 시작해서 시간에 따라 감소
	GameStatComponent->EmptyCurrentValue(EETGameStatType::Momentum);
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

bool AETCharacter::HasEnoughActionCost(const FETCharacterActionMontageData* InMontageData) const
{
	if (InMontageData == nullptr)
	{
		return false;
	}

	if (InMontageData->CostValue <= 0.f)
	{
		return true;
	}

	return GameStatComponent->HasEnoughCurrentValue(InMontageData->CostStatType, InMontageData->CostValue);
}

void AETCharacter::ConsumeActionCost(const FETCharacterActionMontageData* InMontageData)
{
	if (InMontageData && InMontageData->CostValue > 0.f)
	{
		GameStatComponent->AddDepletedValue(InMontageData->CostStatType, -InMontageData->CostValue);
	}
}

const FETCharacterActionMontageData* AETCharacter::FindPlayableActionMontageData(const FGameplayTag& InActionTag, const int32 InIndex) const
{
	if (CachedActionDataAsset == nullptr)
	{
		return nullptr;
	}

	const FETCharacterActionMontageData* MontageData = CachedActionDataAsset->GetActionMontageData(InActionTag, InIndex);
	if (MontageData == nullptr || MontageData->Montage == nullptr || HasEnoughActionCost(MontageData) == false)
	{
		return nullptr;
	}

	return MontageData;
}

bool AETCharacter::CanPlayAction(const FGameplayTag& InActionTag) const
{
	const FETCharacterActionData* ActionData = CachedActionDataAsset ? CachedActionDataAsset->GetActionData(InActionTag) : nullptr;
	if (ActionData == nullptr)
	{
		return true;
	}

	const bool bAllowed = ActionData->AllowedStateContainer.IsEmpty() || CharacterStateComponent->HasCurrentState(ActionData->AllowedStateContainer);
	const bool bBlocked = CharacterStateComponent->HasCurrentState(ActionData->BlockedStateContainer);

	return bAllowed && bBlocked == false;
}

bool AETCharacter::PlayAction(const FGameplayTag& InActionTag, const int32 InIndex, const FGameplayTag& InStateTag, const FOnActionEnded& InOnActionEnded)
{
	if (CanPlayAction(InActionTag) == false)
	{
		return false;
	}

	int32 MontageIndex = InIndex;
	if (MontageIndex == INDEX_NONE && CachedActionDataAsset)
	{
		const int32 MontageNum = CachedActionDataAsset->GetActionMontageDataArray(InActionTag).Num();
		MontageIndex = MontageNum > 0 ? FMath::RandRange(0, MontageNum - 1) : 0;
	}

	return PlayAction(FindPlayableActionMontageData(InActionTag, MontageIndex), InStateTag, InOnActionEnded);
}

bool AETCharacter::PlayAction(const FETCharacterActionMontageData* InMontageData, const FGameplayTag& InStateTag, const FOnActionEnded& InOnActionEnded)
{
	if (InMontageData == nullptr || InMontageData->Montage == nullptr)
	{
		return false;
	}

	UAnimMontage* Montage = InMontageData->Montage;

	// 재생 중이던 몽타주의 BlendingOut 은 PlayAnimMontage 안에서 바로 호출되므로 그 전에 새 행동으로 갱신
	const uint32 CurrentActionSerial = ++ActionSerial;
	CharacterStateComponent->ChangeState(InStateTag);

	if (PlayAnimMontage(Montage) <= 0.f)
	{
		CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Idle);
		return false;
	}

	ConsumeActionCost(InMontageData);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageBlendingOutStarted BlendingOutDelegate;
		BlendingOutDelegate.BindUObject(this, &ThisClass::HandleActionMontageBlendingOut, InStateTag, CurrentActionSerial, InOnActionEnded);
		AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, Montage);
	}

	return true;
}

void AETCharacter::HandleActionMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted, FGameplayTag InStateTag, uint32 InActionSerial, FOnActionEnded InOnActionEnded)
{
	OnActionMontageBlendingOut(InStateTag, bInterrupted);

	// 다른 행동이 이미 시작됐거나 (피격 등으로) 상태가 바뀌었으면 건드리지 않음
	if (InActionSerial == ActionSerial && CharacterStateComponent->IsCurrentState(InStateTag))
	{
		CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Idle);
	}

	InOnActionEnded.ExecuteIfBound(bInterrupted);
}
