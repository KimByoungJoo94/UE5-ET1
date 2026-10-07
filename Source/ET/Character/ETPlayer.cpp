#include "Character/ETPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "DataAsset/ETCharacterActionDataAsset.h"
#include "Components/ETCharacterStateComponent.h"
#include "Components/ETGameStatComponent.h"
#include "Components/ETWeaponCollisionComponent.h"
#include "Components/ETChargeAttackComponent.h"
#include "Components/ETInteractionComponent.h"
#include "Components/ETAfterImageComponent.h"
#include "GameFramework/RootMotionSource.h"
#include "Subsystem/ETTimeDilationSubsystem.h"

namespace
{
	const FName DodgeRootMotionSourceName(TEXT("ETDodge"));
}

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
}

void AETPlayer::BeginPlay()
{
	Super::BeginPlay();

	CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Idle);

	GameStatComponent->StartIncreaseOverTime(EETGameStatType::Mana, EETGameStatType::ManaRegen);

	// 기세는 0 에서 시작해서 시간에 따라 감소
	const float MomentumMaxValue = GameStatComponent->GetGameStat(EETGameStatType::Momentum).GetMaxValue();
	GameStatComponent->AddDepletedValue(EETGameStatType::Momentum, -MomentumMaxValue);
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
	}
}

void AETPlayer::OnMoveActionTriggered(const FInputActionValue& InValue)
{
	if (CanMove() == false)
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
	const FVector2D& LookDirection = InValue.Get<FVector2D>();
	
	AddControllerYawInput(LookDirection.X);
	AddControllerPitchInput(LookDirection.Y);
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
	if (CanHeavyAttack() && ActionDataAsset)
	{		
		if (HeavyAttackMontage = ActionDataAsset->GetAnimMontage(ETGameplayTags::Character_Action_HeavyAttack, 0))
		{
			bPlayHeavyAttack = false;
			CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_HeavyAttacking);

			PlayAnimMontage(HeavyAttackMontage, 1.f, TEXT("Intro"));
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
	InteractionComponent->DoInteraction();
}

void AETPlayer::OnDodgeActionStarted()
{
	if (CanDodge() == false || ActionDataAsset == nullptr)
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

	UAnimMontage* DodgeMontage = ActionDataAsset->GetAnimMontage(ETGameplayTags::Character_Action_Dodge, DodgeMontageIndex);
	if (DodgeMontage == nullptr)
	{
		return;
	}

	WeaponCollisionComponent->EndWeaponCollision();
	ResetComboAttack();
	ResetHeavyAttack();

	CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Dodging);

	if (PlayAnimMontage(DodgeMontage) <= 0.f)
	{
		CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Idle);
		return;
	}

	GameStatComponent->AddDepletedValue(EETGameStatType::Mana, -PlayerDodgeData.DodgeManaCost);

	const FVector DodgeDirection = DodgeMontageIndex == 0 ? GetActorForwardVector() : -GetActorForwardVector();

	TSharedPtr<FRootMotionSource_ConstantForce> DodgeForce = MakeShared<FRootMotionSource_ConstantForce>();
	DodgeForce->InstanceName = DodgeRootMotionSourceName;
	DodgeForce->AccumulateMode = ERootMotionAccumulateMode::Override;
	DodgeForce->Force = DodgeDirection * (PlayerDodgeData.DodgeDistance / PlayerDodgeData.DodgeDuration);
	DodgeForce->Duration = PlayerDodgeData.DodgeDuration;
	DodgeForce->FinishVelocityParams.Mode = ERootMotionFinishVelocityMode::SetVelocity;
	DodgeForce->FinishVelocityParams.SetVelocity = FVector::ZeroVector;
	GetCharacterMovement()->ApplyRootMotionSource(DodgeForce);

	// TODO : 적 공격의 회피 가능 구간에서만 호출
	PlayPerfectDodge();

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageBlendingOutStarted BlendingOutDelegate;
		BlendingOutDelegate.BindUObject(this, &ThisClass::OnDodgeMontageBlendingOut);
		AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, DodgeMontage);
	}
}

void AETPlayer::OnParryActionStarted()
{
	if (CanParry() == false || ActionDataAsset == nullptr)
	{
		return;
	}

	UAnimMontage* NewParryMontage = ActionDataAsset->GetAnimMontage(ETGameplayTags::Character_Action_Parry, 0);
	if (NewParryMontage == nullptr)
	{
		return;
	}

	WeaponCollisionComponent->EndWeaponCollision();
	ResetComboAttack();
	ResetHeavyAttack();
	ResetParry();

	ParryMontage = NewParryMontage;

	CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Parrying);

	if (PlayAnimMontage(ParryMontage) <= 0.f)
	{
		ResetParry();
		CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Idle);
		return;
	}

	GameStatComponent->AddDepletedValue(EETGameStatType::Mana, -PlayerParryData.ParryManaCost);

	if (UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance())
	{
		FOnMontageBlendingOutStarted BlendingOutDelegate;
		BlendingOutDelegate.BindUObject(this, &ThisClass::OnParryMontageBlendingOut);
		AnimInstance->Montage_SetBlendingOutDelegate(BlendingOutDelegate, ParryMontage);
	}
}

void AETPlayer::OnParryActionCompleted()
{
	ReleaseParry();
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

float AETPlayer::GetAttackDamage() const
{
	return Super::GetAttackDamage() * ChargeAttackComponent->GetDamageMultiplier();
}

bool AETPlayer::TryAvoidDamage(AActor* InDamageCauser)
{
	if (Super::TryAvoidDamage(InDamageCauser))
	{
		return true;
	}

	if (CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Dodging))
	{
		// TODO : 퍼펙트 회피 구간이면 PlayPerfectDodge + 기세 획득
		return true;
	}

	if (CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Parrying))
	{
		// TODO : 퍼펙트 패리 구간이면 퍼펙트 패리 + 기세 획득
		return true;
	}

	return false;
}

void AETPlayer::HitReact(AActor* InDamageCauser)
{
	if (IsValid(InDamageCauser) == false || ActionDataAsset == nullptr)
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

	UAnimMontage* HitMontage = ActionDataAsset->GetAnimMontage(ETGameplayTags::Character_Action_Hit, HitMontageIndex);
	if (HitMontage == nullptr)
	{
		HitMontage = ActionDataAsset->GetAnimMontage(ETGameplayTags::Character_Action_Hit, 0);
	}

	if (HitMontage == nullptr)
	{
		return;
	}

	GetCharacterMovement()->RemoveRootMotionSource(DodgeRootMotionSourceName);
	WeaponCollisionComponent->EndWeaponCollision();
	ResetComboAttack();
	ResetHeavyAttack();

	PlayAnimMontage(HitMontage);
}

void AETPlayer::Die()
{
	// TODO
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

bool AETPlayer::CanMove()
{
	FGameplayTagContainer CheckContainer;
	CheckContainer.AddTag(ETGameplayTags::Character_State_Idle);
	CheckContainer.AddTag(ETGameplayTags::Character_State_Attacking);

	return CharacterStateComponent->HasCurrentState(CheckContainer);
}

bool AETPlayer::CanPlayComboAttack()
{
	FGameplayTagContainer CheckContainer;
	CheckContainer.AddTag(ETGameplayTags::Character_State_Death);
	CheckContainer.AddTag(ETGameplayTags::Character_State_HeavyAttacking);

	return CharacterStateComponent->HasCurrentState(CheckContainer) == false;
}

bool AETPlayer::CanDodge()
{
	FGameplayTagContainer CheckContainer;
	CheckContainer.AddTag(ETGameplayTags::Character_State_Death);
	CheckContainer.AddTag(ETGameplayTags::Character_State_Dodging);

	return CharacterStateComponent->HasCurrentState(CheckContainer) == false &&
		GetCharacterMovement()->IsFalling() == false &&
		GameStatComponent->HasEnoughCurrentValue(EETGameStatType::Mana, PlayerDodgeData.DodgeManaCost);
}

bool AETPlayer::CanParry()
{
	FGameplayTagContainer CheckContainer;
	CheckContainer.AddTag(ETGameplayTags::Character_State_Death);
	CheckContainer.AddTag(ETGameplayTags::Character_State_Dodging);
	CheckContainer.AddTag(ETGameplayTags::Character_State_Parrying);

	return CharacterStateComponent->HasCurrentState(CheckContainer) == false &&
		GetCharacterMovement()->IsFalling() == false &&
		GameStatComponent->HasEnoughCurrentValue(EETGameStatType::Mana, PlayerParryData.ParryManaCost);
}

bool AETPlayer::CanHeavyAttack()
{
	FGameplayTagContainer CheckContainer;
	CheckContainer.AddTag(ETGameplayTags::Character_State_Death);
	CheckContainer.AddTag(ETGameplayTags::Character_State_Attacking);
	
	return CharacterStateComponent->HasCurrentState(CheckContainer) == false;
}

void AETPlayer::PlayComboAttack()
{	
	if (CanPlayComboAttack() && ActionDataAsset)
	{
		FGameplayTag AttackGameplayTag;
		if (UCharacterMovementComponent* CharacterMovementComponent = GetCharacterMovement())
		{
			AttackGameplayTag = CharacterMovementComponent->IsFalling() ? 
				ETGameplayTags::Character_Action_JumpAttack : ETGameplayTags::Character_Action_ComboAttack;
		}

		const int32 CurrentIndex = ComboAttackIndex;

		const TArray<TObjectPtr<UAnimMontage>>& AttackMontageArray = ActionDataAsset->GetAnimMontageArray(AttackGameplayTag);
		if (AttackMontageArray.IsEmpty() == false &&
			AttackMontageArray.IsValidIndex(CurrentIndex))
		{
			ComboAttackIndex = (ComboAttackIndex + 1) % AttackMontageArray.Num();
						
			CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Attacking);
			
			PlayAnimMontage(AttackMontageArray[CurrentIndex]);
		}
	}
}

void AETPlayer::OnChargeCountChanged(const int32 InCurrentChargeCount, const int32 InMaxChargeCount)
{
	if (InCurrentChargeCount <= 0)
	{
		return;
	}

	// TODO : 충전 파티클 출력
}

void AETPlayer::OnDodgeMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted)
{
	if (bInterrupted == false)
	{
		CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Idle);
	}
}

void AETPlayer::PlayPerfectDodge()
{
	AfterImageComponent->SpawnAfterImage();

	if (UETTimeDilationSubsystem* TimeDilationSubsystem = GetWorld()->GetSubsystem<UETTimeDilationSubsystem>())
	{
		TimeDilationSubsystem->StartTimeDilation(PlayerDodgeData.PerfectDodgeTimeDilation, PlayerDodgeData.PerfectDodgeSlowDuration);
	}
}

void AETPlayer::OnParryMontageBlendingOut(UAnimMontage* InMontage, bool bInterrupted)
{
	ResetParry();

	if (bInterrupted == false)
	{
		CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Idle);
	}
}
