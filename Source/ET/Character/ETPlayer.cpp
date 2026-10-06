#include "Character/ETPlayer.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "DataAsset/ETCharacterActionDataAsset.h"
#include "Components/ETCharacterStateComponent.h"
#include "Components/ETCombatComponent.h"
#include "Components/ETInteractionComponent.h"

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
}

void AETPlayer::BeginPlay()
{
	Super::BeginPlay();

	CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Idle);
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

	CharacterStateComponent->ChangeState(ETGameplayTags::Character_State_Idle);
}

void AETPlayer::OnStartWeaponCollision()
{
	CombatComponent->StartWeaponCollision();
}

void AETPlayer::OnEndWeaponCollision()
{
	CombatComponent->EndWeaponCollision();
}

void AETPlayer::OnHeavyAttackPause()
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
		}
	}
}

void AETPlayer::OnHit()
{
	//// LookAt 회전값을 구합니다. (현재 Actor가 공격자를 바라보는 회전값)
	//const FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetOwner()->GetActorLocation(), Attacker->GetActorLocation());
	//// 현재 Actor의 회전값과 LookAt 회전값의 차이를 구합니다.
	//const FRotator DeltaRotation = UKismetMathLibrary::NormalizedDeltaRotator(GetOwner()->GetActorRotation(), LookAtRotation);
	//// Z축 기준의 회전값 차이만을 취합니다.
	//const float DeltaZ = DeltaRotation.Yaw;

	//EHitDirection HitDirection = EHitDirection::Front;

	//if (UKismetMathLibrary::InRange_FloatFloat(DeltaZ, -45.f, 45.f))
	//{
	//	HitDirection = EHitDirection::Front;
	//	UE_LOG(LogTemp, Log, TEXT("Front"));
	//}
	//else if (UKismetMathLibrary::InRange_FloatFloat(DeltaZ, 45.f, 135.f))
	//{
	//	HitDirection = EHitDirection::Left;
	//	UE_LOG(LogTemp, Log, TEXT("Left"));
	//}
	//else if (UKismetMathLibrary::InRange_FloatFloat(DeltaZ, 135.f, 180.f)
	//	|| UKismetMathLibrary::InRange_FloatFloat(DeltaZ, -180.f, -135.f))
	//{
	//	HitDirection = EHitDirection::Back;
	//	UE_LOG(LogTemp, Log, TEXT("Back"));
	//}
	//else if (UKismetMathLibrary::InRange_FloatFloat(DeltaZ, -135.f, -45.f))
	//{
	//	HitDirection = EHitDirection::Right;
	//	UE_LOG(LogTemp, Log, TEXT("Right"));
	//}

	//UAnimMontage* SelectedMontage = nullptr;
	//switch (HitDirection)
	//{
	//case EHitDirection::Front:
	//	SelectedMontage = GetMontageForTag(DS1GameplayTags::Character_Action_HitReaction, 0);
	//	break;
	//case EHitDirection::Back:
	//	SelectedMontage = GetMontageForTag(DS1GameplayTags::Character_Action_HitReaction, 1);
	//	break;
	//case EHitDirection::Left:
	//	SelectedMontage = GetMontageForTag(DS1GameplayTags::Character_Action_HitReaction, 2);
	//	break;
	//case EHitDirection::Right:
	//	SelectedMontage = GetMontageForTag(DS1GameplayTags::Character_Action_HitReaction, 3);
	//	break;
	//}

	//return SelectedMontage;
}

void AETPlayer::OnDeath()
{
	// TODO
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