#include "Character/ETPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "Camera/CameraComponent.h"
#include "Camera/CameraShakeBase.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "DataAsset/ETCharacterActionDataAsset.h"
#include "DataAsset/ETCameraShakeTableRowData.h"
#include "Components/ETCharacterStateComponent.h"
#include "Components/ETGameStatComponent.h"
#include "Components/ETAttackCollisionComponent.h"
#include "Components/ETChargeAttackComponent.h"
#include "Components/ETInteractionComponent.h"
#include "Components/ETAfterImageComponent.h"
#include "Components/ETLockOnComponent.h"
#include "Components/ETDamageTextComponent.h"
#include "Subsystem/ETTimeDilationSubsystem.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Common/ETNames.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Engine/DamageEvents.h"

AETPlayer::AETPlayer()
{
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	if (UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement())
	{
		CharacterMovementComponent->bOrientRotationToMovement = true;
		CharacterMovementComponent->RotationRate = FRotator(0.f, 500.f, 0.f);
		CharacterMovementComponent->MaxWalkSpeed = 500.f;
		CharacterMovementComponent->BrakingDecelerationWalking = 2000.f;
	}

	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(RootComponent);
	SpringArmComponent->TargetArmLength = 500.0f;
	SpringArmComponent->SetRelativeRotation(FRotator(-30.f, 0.f, 0.f));
	SpringArmComponent->bUsePawnControlRotation = true;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent);
	CameraComponent->bUsePawnControlRotation = false;

	InteractionComponent = CreateDefaultSubobject<UETInteractionComponent>(TEXT("InteractionComponent"));
	ChargeAttackComponent = CreateDefaultSubobject<UETChargeAttackComponent>(TEXT("ChargeAttackComponent"));
	AfterImageComponent = CreateDefaultSubobject<UETAfterImageComponent>(TEXT("AfterImageComponent"));
	LockOnComponent = CreateDefaultSubobject<UETLockOnComponent>(TEXT("LockOnComponent"));
	DamageTextComponent = CreateDefaultSubobject<UETDamageTextComponent>(TEXT("DamageTextComponent"));
}

void AETPlayer::BeginPlay()
{
	Super::BeginPlay();

	CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Idle);

	ResetGameStat();

	GameStatComponent->StartIncreaseOverTime(EETGameStatType::Mana, EETGameStatType::ManaRegen);
	GameStatComponent->StartDecreaseOverTime(EETGameStatType::Momentum, EETGameStatType::MomentumDrain);

	OnChargeCountChangedHandle = ChargeAttackComponent->OnChargeCountChanged.AddUObject(this, &ThisClass::OnChargeCountChanged);
}

void AETPlayer::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	ChargeAttackComponent->OnChargeCountChanged.Remove(OnChargeCountChangedHandle);

	Super::EndPlay(EndPlayReason);
}

void AETPlayer::NotifyControllerChanged()
{
	Super::NotifyControllerChanged();

	if (APlayerController* PlayerController = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* EnhancedInputLocalPlayerSubsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			EnhancedInputLocalPlayerSubsystem->AddMappingContext(PlayerInputData.InputMappingContext, 0);
		}
	}
}
void AETPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// 패리 유지 중 마나가 모두 소모되면 해제
	if (bParryPaused && GameStatComponent->GetGameStat(EETGameStatType::Mana).GetCurrentValue() <= 0.f)
	{
		ReleaseParry();
	}
}

bool AETPlayer::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation() && CanPlayAction(ETGameplayTags::Character_Action_Jump);
}

void AETPlayer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(PlayerInputData.JumpAction, ETriggerEvent::Started, this, &ThisClass::Jump);
		EnhancedInputComponent->BindAction(PlayerInputData.JumpAction, ETriggerEvent::Completed, this, &ThisClass::StopJumping);

		EnhancedInputComponent->BindAction(PlayerInputData.MoveAction, ETriggerEvent::Triggered, this, &ThisClass::OnMoveActionTriggered);
		EnhancedInputComponent->BindAction(PlayerInputData.LookAction, ETriggerEvent::Triggered, this, &ThisClass::OnLookActionTriggered);
		EnhancedInputComponent->BindAction(PlayerInputData.AttackAction, ETriggerEvent::Completed, this, &ThisClass::OnAttackActionCompleted);

		EnhancedInputComponent->BindAction(PlayerInputData.HeavyAttackAction, ETriggerEvent::Started, this, &ThisClass::OnHeavyAttackActionStarted);
		EnhancedInputComponent->BindAction(PlayerInputData.HeavyAttackAction, ETriggerEvent::Completed, this, &ThisClass::OnHeavyAttackActionCompleted);

		EnhancedInputComponent->BindAction(PlayerInputData.PrimaryAction, ETriggerEvent::Completed, this, &ThisClass::OnPrimaryActionCompleted);

		EnhancedInputComponent->BindAction(PlayerInputData.DodgeAction, ETriggerEvent::Started, this, &ThisClass::OnDodgeActionStarted);
		EnhancedInputComponent->BindAction(PlayerInputData.ParryAction, ETriggerEvent::Started, this, &ThisClass::OnParryActionStarted);
		EnhancedInputComponent->BindAction(PlayerInputData.ParryAction, ETriggerEvent::Completed, this, &ThisClass::OnParryActionCompleted);

		EnhancedInputComponent->BindAction(PlayerInputData.UseItemAction, ETriggerEvent::Started, this, &ThisClass::OnUseItemActionStarted);
		EnhancedInputComponent->BindAction(PlayerInputData.LockOnAction, ETriggerEvent::Started, this, &ThisClass::OnLockOnActionStarted);

		for (int32 Index = 0; Index < PlayerInputData.ActiveSkillActionArray.Num(); ++Index)
		{
			if (PlayerInputData.ActiveSkillActionArray[Index])
			{
				EnhancedInputComponent->BindAction(PlayerInputData.ActiveSkillActionArray[Index], ETriggerEvent::Started, this, &ThisClass::OnActiveSkillActionStarted, Index);
			}
		}
	}
}

void AETPlayer::OnMoveActionTriggered(const FInputActionValue& InValue)
{
	if (CanPlayAction(ETGameplayTags::Character_Action_Move) == false)
	{
		return;
	}

	const FVector2D &MovementVector = InValue.Get<FVector2D>();

	if (Controller)
	{
		const FRotator& Rotation = Controller->GetControlRotation();
		const FRotator YawRotator(0, Rotation.Yaw, 0);

		const FVector& ForwardVector = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::X);
		const FVector& RightVector = FRotationMatrix(YawRotator).GetUnitAxis(EAxis::Y);
				
		AddMovementInput(ForwardVector, MovementVector.Y);
		AddMovementInput(RightVector, MovementVector.X);
	}
}

void AETPlayer::OnLookActionTriggered(const FInputActionValue& InValue)
{
	// 락온 중에는 카메라가 대상을 따라가므로 시점 입력 무시
	if (LockOnComponent->IsLockedOn())
	{
		return;
	}

	const FVector2D& LookDirection = InValue.Get<FVector2D>();
	
	AddControllerYawInput(LookDirection.X);
	AddControllerPitchInput(LookDirection.Y);
}

void AETPlayer::OnLockOnActionStarted()
{
	LockOnComponent->ToggleLockOn();
}

void AETPlayer::OnAttackActionCompleted()
{
	if (CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Attacking))
	{
		bComboAttackReserved = true;
	}
	else
	{	
		PlayComboAttack();
	}
}

void AETPlayer::OnHeavyAttackActionStarted()
{
	if (CanPlayAction(ETGameplayTags::Character_Action_HeavyAttack) && CachedActionDataAsset)
	{
		const FETCharacterActionMontageData* HeavyAttackMontageData = CachedActionDataAsset->GetActionMontageData(ETGameplayTags::Character_Action_HeavyAttack, 0);
		if (HeavyAttackMontageData && HeavyAttackMontageData->Montage && HasEnoughActionCost(HeavyAttackMontageData))
		{
			HeavyAttackMontage = HeavyAttackMontageData->Montage;
			bPlayHeavyAttack = false;
			CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_HeavyAttacking);

			if (PlayAnimMontage(HeavyAttackMontage, 1.f, TEXT("Intro")) > 0.f)
			{
				ConsumeActionCost(HeavyAttackMontageData);
			}
		}
	}
}

void AETPlayer::OnHeavyAttackActionCompleted()
{	
	if (CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_HeavyAttacking) == false)
	{
		return;
	}

	if (bPlayHeavyAttack)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Resume(HeavyAttackMontage);
			ChargeAttackComponent->EndChargeAttack();
		}
	}
	else
	{
		bPlayHeavyAttack = true;
	}
}

void AETPlayer::OnPrimaryActionCompleted()
{
	if (CanPlayAction(ETGameplayTags::Character_Action_Interaction) == false ||
		GetCharacterMovement()->IsFalling() ||
		InteractionComponent->GetInteractionTarget() == nullptr)
	{
		return;
	}

	PlayAction(ETGameplayTags::Character_Action_Interaction, 0, ETGameplayTags::Character_State_Interacting);
}

void AETPlayer::OnUseItemActionStarted()
{
	if (CanPlayAction(ETGameplayTags::Character_Action_UseItem) == false || GetCharacterMovement()->IsFalling())
	{
		return;
	}

	// 실제 아이템 사용은 몽타주의 ETAnimNotify_UseItem 시점에 처리
	PlayAction(ETGameplayTags::Character_Action_UseItem, 0, ETGameplayTags::Character_State_UsingItem);
}

void AETPlayer::UseItem()
{
	// TODO : 기본 체력 회복 또는 설정한 아이템 사용
	GameStatComponent->AddDepletedValue(EETGameStatType::Health, 10.f);
}

void AETPlayer::OnDodgeActionStarted()
{
	if (CanPlayAction(ETGameplayTags::Character_Action_Dodge) == false || GetCharacterMovement()->IsFalling())
	{
		return;
	}

	// Character.Action.Dodge 몽타주 배열 인덱스 (0 : Fwd, 1 : Bwd)
	int32 DodgeMontageIndex = 1;

	const FVector InputDirection = GetLastMovementInputVector().GetSafeNormal2D();
	if (InputDirection.IsNearlyZero() == false)
	{
		const bool bForward = FVector::DotProduct(GetActorForwardVector(), InputDirection) >= 0.f;

		DodgeMontageIndex = bForward ? 0 : 1;
		SetActorRotation((bForward ? InputDirection : -InputDirection).Rotation());
	}

	const FETCharacterActionMontageData* DodgeMontageData = FindPlayableActionMontageData(ETGameplayTags::Character_Action_Dodge, DodgeMontageIndex);
	if (DodgeMontageData == nullptr)
	{
		return;
	}

	AttackCollisionComponent->EndWeaponCollision();
	ResetComboAttack();
	ResetHeavyAttack();

	if (PlayAction(DodgeMontageData, ETGameplayTags::Character_State_Dodging) == false)
	{
		return;
	}

	const FVector DodgeDirection = DodgeMontageIndex == 0 ? GetActorForwardVector() : -GetActorForwardVector();

	TSharedPtr<FRootMotionSource_ConstantForce> DodgeForce = MakeShared<FRootMotionSource_ConstantForce>();
	DodgeForce->InstanceName = ETNames::RootMotionSource::Dodge;
	DodgeForce->AccumulateMode = ERootMotionAccumulateMode::Override;
	DodgeForce->Force = DodgeDirection * (PlayerDodgeData.DodgeDistance / PlayerDodgeData.DodgeDuration);
	DodgeForce->Duration = PlayerDodgeData.DodgeDuration;
	DodgeForce->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	DodgeForce->FinishVelocityParams.SetVelocity = FVector::ZeroVector;
	GetCharacterMovement()->ApplyRootMotionSource(DodgeForce);
}

void AETPlayer::OnParryActionStarted()
{
	if (CanPlayAction(ETGameplayTags::Character_Action_Parry) == false || GetCharacterMovement()->IsFalling())
	{
		return;
	}

	const FETCharacterActionMontageData* ParryMontageData = FindPlayableActionMontageData(ETGameplayTags::Character_Action_Parry, 0);
	if (ParryMontageData == nullptr)
	{
		return;
	}

	AttackCollisionComponent->EndWeaponCollision();
	ResetComboAttack();
	ResetHeavyAttack();
	ResetParry();

	ParryMontage = ParryMontageData->Montage;

	if (PlayAction(ParryMontageData, ETGameplayTags::Character_State_Parrying) == false)
	{
		ResetParry();
	}
}

void AETPlayer::OnParryActionCompleted()
{
	ReleaseParry();
}

void AETPlayer::OnActiveSkillActionStarted(const int32 InSkillIndex)
{
	switch (InSkillIndex)
	{
	case 0:
	case 1:
	case 2:
		PlayActiveSkill(InSkillIndex);
		break;

	case 3:
		if (TryPayActiveSkillCost(InSkillIndex))
		{
			// TODO : 스킬 D
		}
		break;

	default:
		break;
	}
}

bool AETPlayer::TryPayActiveSkillCost(const int32 InSkillIndex)
{
	// 몽타주 없는 스킬용 : 몽타주 항목의 비용만 확인 후 차감
	if (CanPlayAction(ETGameplayTags::Character_Action_ActiveSkill) == false || GetCharacterMovement()->IsFalling() || CachedActionDataAsset == nullptr)
	{
		return false;
	}

	const FETCharacterActionMontageData* SkillMontageData = CachedActionDataAsset->GetActionMontageData(ETGameplayTags::Character_Action_ActiveSkill, InSkillIndex);
	if (SkillMontageData == nullptr || HasEnoughActionCost(SkillMontageData) == false)
	{
		return false;
	}

	ConsumeActionCost(SkillMontageData);
	return true;
}

void AETPlayer::PlayActiveSkill(const int32 InSkillIndex)
{
	if (CanPlayAction(ETGameplayTags::Character_Action_ActiveSkill) == false || GetCharacterMovement()->IsFalling())
	{
		return;
	}

	// Character.Action.ActiveSkill 몽타주 배열 인덱스 = 스킬 슬롯 인덱스
	const FETCharacterActionMontageData* SkillMontageData = FindPlayableActionMontageData(ETGameplayTags::Character_Action_ActiveSkill, InSkillIndex);
	if (SkillMontageData == nullptr)
	{
		return;
	}

	AttackCollisionComponent->EndWeaponCollision();
	ResetComboAttack();
	ResetHeavyAttack();

	PlayAction(SkillMontageData, ETGameplayTags::Character_State_UsingSkill);
}

void AETPlayer::AdvanceComboAttack()
{
	if (bComboAttackReserved)
	{
		bComboAttackReserved = false;
		PlayComboAttack();
	}
}

void AETPlayer::ResetComboAttack()
{
	bComboAttackReserved = false;	
	ComboAttackIndex = 0;

	CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Idle);
}

void AETPlayer::ResetHeavyAttack()
{
	bPlayHeavyAttack = false;
	HeavyAttackMontage = nullptr;
	ChargeAttackComponent->ResetChargeAttack();

	CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Idle);
}

void AETPlayer::PauseHeavyAttack()
{
	if (HeavyAttackMontage == nullptr ||
		CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_HeavyAttacking) == false)
	{
		return;
	}

	if (bPlayHeavyAttack == false)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Pause(HeavyAttackMontage);
			bPlayHeavyAttack = true;
			ChargeAttackComponent->StartChargeAttack();
		}
	}
}

void AETPlayer::PauseParry()
{
	if (ParryMontage == nullptr || bParryReleased || bParryPaused ||
		CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Parrying) == false)
	{
		return;
	}

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		AnimInstance->Montage_Pause(ParryMontage);
		bParryPaused = true;

		GameStatComponent->StartDecreaseOverTime(EETGameStatType::Mana, EETGameStatType::ManaDrain);
	}
}

void AETPlayer::ReleaseParry()
{
	if (ParryMontage == nullptr ||
		CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Parrying) == false)
	{
		return;
	}

	// 멈춤 노티파이 전에 키를 떼면 멈추지 않고 그대로 복구 자세까지 재생
	bParryReleased = true;

	if (bParryPaused)
	{
		if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
		{
			AnimInstance->Montage_Resume(ParryMontage);
		}

		bParryPaused = false;

		GameStatComponent->StartIncreaseOverTime(EETGameStatType::Mana, EETGameStatType::ManaRegen);
	}
}

void AETPlayer::ResetParry()
{
	// 패리 유지 중 끊긴 경우 (회피, 피격 등) 마나 회복으로 복구
	if (bParryPaused)
	{
		GameStatComponent->StartIncreaseOverTime(EETGameStatType::Mana, EETGameStatType::ManaRegen);
	}

	bParryPaused = false;
	bParryReleased = false;
	ParryMontage = nullptr;
}

void AETPlayer::SetPerfectTiming(const bool bInPerfectTiming)
{
	bPerfectTiming = bInPerfectTiming;
}

void AETPlayer::GainPerfectMomentum()
{
	GameStatComponent->AddDepletedValue(EETGameStatType::Momentum, GameStatComponent->GetGameStat(EETGameStatType::MomentumGain).GetCurrentValue());
}

float AETPlayer::AttackTarget(AActor* InTarget, const FHitResult& InHitResult)
{
	const float ActualDamage = Super::AttackTarget(InTarget, InHitResult);
	if (ActualDamage > 0.f)
	{
		DamageTextComponent->ShowDamageText(InHitResult.ImpactPoint, ActualDamage);
		PlayCameraShake(ETNames::RowName::CameraShakeHit);
	}

	return ActualDamage;
}

float AETPlayer::GetAttackDamage() const
{
	return Super::GetAttackDamage() * ChargeAttackComponent->GetDamageMultiplier();
}

AActor* AETPlayer::GetAttackTarget() const
{
	// 락온 중에만 공격 방향 보정
	return LockOnComponent->GetLockOnTarget();
}

bool AETPlayer::TryAvoidDamage(const FDamageEvent& InDamageEvent, AActor* InDamageCauser)
{
	if (Super::TryAvoidDamage(InDamageEvent, InDamageCauser))
	{
		return true;
	}

	if (CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Dodging))
	{
		// 한 번의 회피에서 여러 공격이 닿아도 퍼펙트는 1회만
		if (bPerfectTiming)
		{
			bPerfectTiming = false;
			PlayPerfectDodge();
			GainPerfectMomentum();
		}
		return true;
	}

	if (CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Parrying))
	{
		const bool bPerfect = bPerfectTiming;
		if (bPerfect)
		{
			bPerfectTiming = false;
			GainPerfectMomentum();
		}

		PlayParryParticle(InDamageEvent, bPerfect);
		PlayCameraShake(bPerfect ? ETNames::RowName::CameraShakePerfectParry : ETNames::RowName::CameraShakeParry);

		const float HitStopDuration = bPerfect ? PlayerParryData.PerfectParryHitStopDuration : PlayerParryData.ParryHitStopDuration;
		StartHitStop(HitStopDuration, PlayerParryData.HitStopTimeDilation);
		if (AETCharacter* AttackerCharacter = Cast<AETCharacter>(InDamageCauser))
		{
			AttackerCharacter->StartHitStop(HitStopDuration, PlayerParryData.HitStopTimeDilation);
		}
		return true;
	}

	return false;
}

void AETPlayer::HitReact(AActor* InDamageCauser)
{
	if (IsValid(InDamageCauser) == false || CachedActionDataAsset == nullptr)
	{
		return;
	}

	// 스킬 시전 중에는 슈퍼아머 (데미지만 받고 피격 리액션 없음)
	if (CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_UsingSkill))
	{
		return;
	}

	const FRotator LookAtRotation = (InDamageCauser->GetActorLocation() - GetActorLocation()).Rotation();
	const float DeltaYaw = (GetActorRotation() - LookAtRotation).GetNormalized().Yaw;

	// Character.Action.Hit 몽타주 배열 인덱스 (0 : Front, 1 : Back, 2 : Left, 3 : Right)
	int32 HitMontageIndex = 0;

	if (FMath::Abs(DeltaYaw) > 135.f)
	{
		HitMontageIndex = 1;
	}
	else if (DeltaYaw > 45.f)
	{
		HitMontageIndex = 2;
	}
	else if (DeltaYaw < -45.f)
	{
		HitMontageIndex = 3;
	}

	UAnimMontage* HitMontage = CachedActionDataAsset->GetAnimMontage(ETGameplayTags::Character_Action_Hit, HitMontageIndex);
	if (HitMontage == nullptr)
	{
		HitMontage = CachedActionDataAsset->GetAnimMontage(ETGameplayTags::Character_Action_Hit, 0);
	}

	if (HitMontage == nullptr)
	{
		return;
	}

	GetCharacterMovement()->RemoveRootMotionSource(ETNames::RootMotionSource::Dodge);
	AttackCollisionComponent->EndWeaponCollision();
	ResetComboAttack();
	ResetHeavyAttack();

	PlayAnimMontage(HitMontage);
}

void AETPlayer::Die()
{
	GetCharacterMovement()->RemoveRootMotionSource(ETNames::RootMotionSource::Dodge);
	ResetComboAttack();
	ResetHeavyAttack();
	ResetParry();
	bPerfectTiming = false;
	LockOnComponent->EndLockOn();

	Super::Die();
}

const FText& AETPlayer::GetPrimaryActionKeyText() const
{
	const APlayerController* PlayerController = Cast<APlayerController>(GetController());
	if (PlayerController == nullptr)
	{
		return FText::GetEmpty();
	}

	const ULocalPlayer* LocalPlayer = PlayerController->GetLocalPlayer();
	if (LocalPlayer == nullptr)
	{
		return FText::GetEmpty();
	}

	const UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>();

	if (InputSubsystem == nullptr || PlayerInputData.PrimaryAction == nullptr)
	{
		return FText::GetEmpty();
	}

	const TArray<FKey> Keys = InputSubsystem->QueryKeysMappedToAction(PlayerInputData.PrimaryAction);

	if (Keys.IsEmpty())
	{
		return FText::GetEmpty();
	}

	return Keys[0].GetDisplayName();
}

void AETPlayer::PlayComboAttack()
{	
	const FGameplayTag AttackGameplayTag = GetCharacterMovement()->IsFalling() ?
		ETGameplayTags::Character_Action_JumpAttack : ETGameplayTags::Character_Action_ComboAttack;

	if (CanPlayAction(AttackGameplayTag) && CachedActionDataAsset)
	{
		const int32 CurrentIndex = ComboAttackIndex;

		const TArray<FETCharacterActionMontageData>& AttackMontageDataArray = CachedActionDataAsset->GetActionMontageDataArray(AttackGameplayTag);
		if (AttackMontageDataArray.IsValidIndex(CurrentIndex) &&
			HasEnoughActionCost(&AttackMontageDataArray[CurrentIndex]))
		{
			ComboAttackIndex = (ComboAttackIndex + 1) % AttackMontageDataArray.Num();

			CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Attacking);

			if (PlayAnimMontage(AttackMontageDataArray[CurrentIndex].Montage) > 0.f)
			{
				ConsumeActionCost(&AttackMontageDataArray[CurrentIndex]);
			}
		}
	}
}

void AETPlayer::OnChargeCountChanged(const int32 InCurrentChargeCount, const int32 InMaxChargeCount)
{
	if (InCurrentChargeCount <= 0)
	{
		return;
	}

	// 충전 단계가 오를 때마다 범위 공격 (판정, 파티클은 범위 공격 테이블 행 기준)
	AttackCollisionComponent->StartRadialAttackCollision(ETNames::RowName::RadialAttackCharge);
}

void AETPlayer::PlayCameraShake(const FName InCameraShakeRowName)
{
	APlayerController* PlayerController = Cast<APlayerController>(GetController());
	const FETCameraShakeTableRowData* RowData = UETGameDataSubsystem::GetRow<FETCameraShakeTableRowData>(this, InCameraShakeRowName);
	if (PlayerController == nullptr || RowData == nullptr || RowData->Scale <= 0.f)
	{
		return;
	}

	if (const TSubclassOf<UCameraShakeBase> CameraShakeClass = RowData->CameraShakeClass.LoadSynchronous())
	{
		PlayerController->ClientStartCameraShake(CameraShakeClass, RowData->Scale);
	}
}

void AETPlayer::PlayParryParticle(const FDamageEvent& InDamageEvent, const bool bInPerfect)
{
	UParticleSystem* ParryParticle = bInPerfect ? PlayerParryData.PerfectParryParticle.LoadSynchronous() : PlayerParryData.ParryParticle.LoadSynchronous();
	if (ParryParticle == nullptr)
	{
		return;
	}

	// 포인트 데미지면 무기가 닿은 지점, 아니면 캐릭터 위치
	FVector ParryLocation = GetActorLocation();
	if (InDamageEvent.IsOfType(FPointDamageEvent::ClassID))
	{
		ParryLocation = static_cast<const FPointDamageEvent&>(InDamageEvent).HitInfo.ImpactPoint;
	}

	UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ParryParticle, ParryLocation);
}

void AETPlayer::PlayPerfectDodge()
{
	AfterImageComponent->SpawnAfterImage();

	if (UETTimeDilationSubsystem* TimeDilationSubsystem = GetWorld()->GetSubsystem<UETTimeDilationSubsystem>())
	{
		TimeDilationSubsystem->StartTimeDilation(PlayerDodgeData.PerfectDodgeTimeDilation, PlayerDodgeData.PerfectDodgeSlowDuration);
	}
}

void AETPlayer::OnActionMontageBlendingOut(const FGameplayTag& InStateTag, const bool bInterrupted)
{
	Super::OnActionMontageBlendingOut(InStateTag, bInterrupted);

	// 패리는 끊긴 경우에도 유지 상태(마나 소모) 정리
	if (InStateTag == ETGameplayTags::Character_State_Parrying)
	{
		ResetParry();
	}
}

