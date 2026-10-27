#include "Character/ETCharacter.h"
#include "Components/ETCharacterStateComponent.h"
#include "Components/ETGameStatComponent.h"
#include "Components/ETAttackCollisionComponent.h"
#include "Components/ETTimeFreezeComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "DataAsset/ETCharacterActionDataAsset.h"
#include "DataAsset/ETCharacterTableRowData.h"
#include "DataAsset/ETActionConditionTableRowData.h"
#include "DataAsset/ETRewardTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Subsystem/ETLevelSubsystem.h"
#include "Subsystem/ETTimeDilationSubsystem.h"
#include "Animation/AnimInstance.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Engine/DamageEvents.h"
#include "Kismet/GameplayStatics.h"
#include "TimerManager.h"

AETCharacter::AETCharacter()
{
	PrimaryActorTick.bCanEverTick = false;

	CharacterStateComponent = CreateDefaultSubobject<UETCharacterStateComponent>(TEXT("CharacterStateComponent"));
	GameStatComponent = CreateDefaultSubobject<UETGameStatComponent>(TEXT("GameStatComponent"));	
	AttackCollisionComponent = CreateDefaultSubobject<UETAttackCollisionComponent>(TEXT("AttackCollisionComponent"));
	TimeFreezeComponent = CreateDefaultSubobject<UETTimeFreezeComponent>(TEXT("TimeFreezeComponent"));

	// 경로 이동(AI MoveTo)도 가속도로 처리 (기본값이면 경로 이동 중 CurrentAcceleration 이 0 이라 애님이 이동으로 전환되지 않음)
	GetCharacterMovement()->GetNavMovementProperties()->bUseAccelerationForPaths = true;

	// 카메라 스프링암이 캐릭터에 걸려 당겨지지 않도록 (지형에만 반응)
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
}

void AETCharacter::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	const UETGameDataSubsystem* GameDataSubsystem = ETSubsystem::GetPtr<UETGameDataSubsystem>(this);
	if (GameDataSubsystem == nullptr)
	{
		return;
	}

	const FETCharacterTableRowData* CharacterRowData = GameDataSubsystem->GetRow<FETCharacterTableRowData>(CharacterRowName);
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
		ProcessDefeat(EventInstigator ? EventInstigator->GetPawn() : DamageCauser);
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

void AETCharacter::HitReact(AActor* InDamageCauser)
{
	if (IsValid(InDamageCauser) == false || CanPlayAction(ETGameplayTags::Character_Action_Hit) == false)
	{
		return;
	}

	// 재생할 몽타주가 있을 때만 진행 중이던 행동 정리 (정리 중 상태가 Idle 로 바뀌므로 재생 전에 처리)
	const FETCharacterActionMontageData* HitMontageData = FindPlayableActionMontageData(ETGameplayTags::Character_Action_Hit, GetHitMontageIndex(InDamageCauser));
	if (HitMontageData == nullptr)
	{
		return;
	}

	AttackCollisionComponent->EndWeaponCollision();
	OnHitReact();

	PlayAction(HitMontageData, ETGameplayTags::Character_State_Hit);
}

int32 AETCharacter::GetHitMontageIndex(const AActor* InDamageCauser) const
{
	const FRotator LookAtRotation = (InDamageCauser->GetActorLocation() - GetActorLocation()).Rotation();
	const float DeltaYaw = (GetActorRotation() - LookAtRotation).GetNormalized().Yaw;

	// 0 : Front, 1 : Back, 2 : Left, 3 : Right
	int32 DirectionIndex = 0;

	if (FMath::Abs(DeltaYaw) > 135.f)
	{
		DirectionIndex = 1;
	}
	else if (DeltaYaw > 45.f)
	{
		DirectionIndex = 2;
	}
	else if (DeltaYaw < -45.f)
	{
		DirectionIndex = 3;
	}

	return DirectionIndex;
}

void AETCharacter::ApplyReward(const FName InRewardRowName)
{
	if (InRewardRowName.IsNone())
	{
		return;
	}

	const FETRewardTableRowData* RewardRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETRewardTableRowData>(InRewardRowName);
	if (RewardRowData == nullptr)
	{
		return;
	}

	for (const TPair<EETGameStatType, float>& PermanentGameStatPair : RewardRowData->PermanentGameStatMap)
	{
		GameStatComponent->AddPermanentValue(PermanentGameStatPair.Key, PermanentGameStatPair.Value);
	}
}

void AETCharacter::ProcessDefeat(AActor* InKiller)
{
	const FETCharacterTableRowData* CharacterRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETCharacterTableRowData>(CharacterRowName);
	if (CharacterRowData == nullptr)
	{
		return;
	}

	const FETCharacterDefeatData& DefeatData = CharacterRowData->DefeatData;
	const bool bBoss = CharacterRowData->CharacterType == EETCharacterType::Boss;
	const bool bRecordDefeat = bBoss || DefeatData.OnceRewardRowName.IsNone() == false;

	if (DefeatData.SlowDuration > 0.f)
	{
		ETSubsystem::Get<UETTimeDilationSubsystem>(this).StartGlobalTimeDilation(DefeatData.SlowTimeDilation, DefeatData.SlowDuration);
	}

	UETLevelSubsystem& LevelSubsystem = ETSubsystem::Get<UETLevelSubsystem>(this);

	if (AETCharacter* Killer = Cast<AETCharacter>(InKiller))
	{
		Killer->ApplyReward(DefeatData.RepeatRewardRowName);

		if (bRecordDefeat && LevelSubsystem.IsCharacterDefeated(CharacterRowName) == false)
		{
			Killer->ApplyReward(DefeatData.OnceRewardRowName);
		}
	}

	// 최초 처치 저장에 보상이 포함되도록 보상 적용 후 기록
	if (bRecordDefeat)
	{
		LevelSubsystem.RecordCharacterDefeat(CharacterRowName, bBoss);
	}
}

void AETCharacter::Die()
{
	AttackCollisionComponent->EndWeaponCollision();

	GetCharacterMovement()->StopMovementImmediately();
	GetCharacterMovement()->DisableMovement();

	// 사망 후에는 맞지도 길을 막지도 않도록
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 시간 정지 중이면 풀어서 사망 몽타주가 재생되도록
	TimeFreezeComponent->Unfreeze();

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

void AETCharacter::ApplyStunDamage(const float InStunDamage)
{
	if (InStunDamage <= 0.f ||
		CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Death) ||
		CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Stun) ||
		CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Intro))
	{
		return;
	}

	// 0 이면 스턴 면역(최대값 0)이거나 다음 틱 스턴 대기 중
	if (GameStatComponent->GetGameStat(EETGameStatType::StunPoint).GetCurrentValue() <= 0.f)
	{
		return;
	}

	GameStatComponent->AddDepletedValue(EETGameStatType::StunPoint, -InStunDamage);

	// 공격 판정 처리 중에 호출되므로 무기 충돌, 몽타주 정리는 다음 틱에 처리
	if (GameStatComponent->GetGameStat(EETGameStatType::StunPoint).GetCurrentValue() <= 0.f)
	{
		GetWorldTimerManager().SetTimerForNextTick(this, &ThisClass::Stun);
	}
}

void AETCharacter::Stun()
{
	if (CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Death))
	{
		return;
	}

	AttackCollisionComponent->EndWeaponCollision();
	GetCharacterMovement()->StopMovementImmediately();

	if (PlayAction(ETGameplayTags::Character_Action_Stun, 0, ETGameplayTags::Character_State_Stun, FOnActionEnded::CreateUObject(this, &ThisClass::OnStunEnded)) == false)
	{
		OnStunEnded(true);
	}
}

void AETCharacter::OnStunEnded(bool bInterrupted)
{
	if (CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Death))
	{
		return;
	}

	GameStatComponent->ResetDepletedValue(EETGameStatType::StunPoint);
}

float AETCharacter::AttackTarget(AActor* InTarget, const FHitResult& InHitResult, const float InDamage)
{
	// 대상의 TakeDamage 반환값 (포인트 데미지로 충돌 정보를 대상에게 전달)
	const FVector HitFromDirection = (InTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal();
	return UGameplayStatics::ApplyPointDamage(InTarget, InDamage, HitFromDirection, InHitResult, GetController(), this, UDamageType::StaticClass());
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

const FETActionConditionTableRowData* AETCharacter::GetActionConditionRowData(const FETCharacterActionMontageData* InMontageData) const
{
	if (InMontageData == nullptr || InMontageData->ActionConditionRowName.IsNone())
	{
		return nullptr;
	}

	return ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETActionConditionTableRowData>(InMontageData->ActionConditionRowName);
}

bool AETCharacter::IsActionConditionMet(const FETCharacterActionMontageData* InMontageData) const
{
	if (InMontageData == nullptr)
	{
		return false;
	}

	const FETActionConditionTableRowData* ConditionRowData = GetActionConditionRowData(InMontageData);
	if (ConditionRowData == nullptr)
	{
		return true;
	}

	if (ConditionRowData->CostValue > 0.f && GameStatComponent->HasEnoughCurrentValue(ConditionRowData->CostStatType, ConditionRowData->CostValue) == false)
	{
		return false;
	}

	if (const double* CooldownEndTime = ActionCooldownEndTimeMap.Find(InMontageData->ActionConditionRowName))
	{
		return GetWorld()->GetTimeSeconds() >= *CooldownEndTime;
	}

	return true;
}

void AETCharacter::ApplyActionCondition(const FETCharacterActionMontageData* InMontageData)
{
	const FETActionConditionTableRowData* ConditionRowData = GetActionConditionRowData(InMontageData);
	if (ConditionRowData == nullptr)
	{
		return;
	}

	if (ConditionRowData->CostValue > 0.f)
	{
		GameStatComponent->AddDepletedValue(ConditionRowData->CostStatType, -ConditionRowData->CostValue);
	}

	if (ConditionRowData->Cooldown > 0.f)
	{
		ActionCooldownEndTimeMap.Emplace(InMontageData->ActionConditionRowName, GetWorld()->GetTimeSeconds() + ConditionRowData->Cooldown);
	}
}

const FETCharacterActionMontageData* AETCharacter::FindPlayableActionMontageData(const FGameplayTag& InActionTag, const int32 InIndex) const
{
	if (CachedActionDataAsset == nullptr)
	{
		return nullptr;
	}

	const FETCharacterActionMontageData* MontageData = CachedActionDataAsset->GetActionMontageData(InActionTag, InIndex);
	if (MontageData == nullptr || MontageData->Montage == nullptr || IsActionConditionMet(MontageData) == false)
	{
		return nullptr;
	}

	return MontageData;
}

int32 AETCharacter::GetActionMontageNum(const FGameplayTag& InActionTag) const
{
	return CachedActionDataAsset ? CachedActionDataAsset->GetActionMontageDataArray(InActionTag).Num() : 0;
}

bool AETCharacter::CanPlayAction(const FGameplayTag& InActionTag) const
{
	// 인트로는 특수 행동이라 액션마다 데이터에 넣지 않고 여기서 일괄 차단
	if (CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Intro) && InActionTag != ETGameplayTags::Character_Action_Death)
	{
		return false;
	}

	const FETCharacterActionData* ActionData = CachedActionDataAsset ? CachedActionDataAsset->GetActionData(InActionTag) : nullptr;
	if (ActionData == nullptr)
	{
		return true;
	}

	const bool bAllowed = ActionData->AllowedStateContainer.IsEmpty() || CharacterStateComponent->HasCurrentState(ActionData->AllowedStateContainer);
	const bool bBlocked = CharacterStateComponent->HasCurrentState(ActionData->BlockedStateContainer);

	return bAllowed && bBlocked == false;
}

bool AETCharacter::PlayIntro()
{
	return PlayAction(ETGameplayTags::Character_Action_Intro, 0, ETGameplayTags::Character_State_Intro, FOnActionEnded::CreateUObject(this, &ThisClass::OnIntroEnded));
}

bool AETCharacter::PlayAction(const FGameplayTag& InActionTag, const int32 InIndex, const FGameplayTag& InStateTag, const FOnActionEnded& InOnActionEnded)
{
	if (CanPlayAction(InActionTag) == false)
	{
		return false;
	}

	int32 MontageIndex = InIndex;
	if (MontageIndex == INDEX_NONE)
	{
		const int32 MontageNum = GetActionMontageNum(InActionTag);
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

	ApplyActionCondition(InMontageData);

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
