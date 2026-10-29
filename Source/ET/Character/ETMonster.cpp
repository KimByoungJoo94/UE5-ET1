#include "Character/ETMonster.h"
#include "Components/WidgetComponent.h"
#include "Components/ETDissolveComponent.h"
#include "Components/ETCharacterStateComponent.h"
#include "Common/ETGameplayTags.h"
#include "UI/ETStatListWidget.h"
#include "Common/ETCommonSettings.h"
#include "Common/ETNames.h"
#include "Controller/ETAIController.h"
#include "Spawn/ETSpawnVolume.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "BrainComponent.h"
#include "Subsystem/ETJobSubsystem.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "DataAsset/ETCharacterTableRowData.h"
#include "Job/ETWidgetJobs.h"
#include "Kismet/GameplayStatics.h"

AETMonster::AETMonster()
{
	AIControllerClass = AETAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 추적, 귀환은 이동 방향으로 회전 (공격 시 회전은 공격 태스크에서 처리)
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	StatBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("StatBarWidgetComponent"));
	StatBarWidgetComponent->SetupAttachment(GetRootComponent());
	StatBarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 150.f));
	StatBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	StatBarWidgetComponent->SetWidgetClass(UETStatListWidget::StaticClass());

	LockOnWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockOnWidgetComponent"));
	// 애니메이션에 따라 몸에 붙어 움직이도록 메시 본에 부착
	LockOnWidgetComponent->SetupAttachment(GetMesh(), ETNames::Socket::LockOn);
	LockOnWidgetComponent->SetDrawSize(FVector2D(40.f, 40.f));
	LockOnWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	LockOnWidgetComponent->SetDrawAtDesiredSize(false);

	DissolveComponent = CreateDefaultSubobject<UETDissolveComponent>(TEXT("DissolveComponent"));
}

void AETMonster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 위젯 컴포넌트가 BeginPlay 에서 위젯을 생성하므로 그 전에 지정 (BP 에서 지정한 값이 우선)
	if (LockOnWidgetComponent->GetWidgetClass() == nullptr)
	{
		LockOnWidgetComponent->SetWidgetClass(GetDefault<UETCommonSettings>()->UI.LockOnWidgetClass.LoadSynchronous());
	}

	DissolveComponent->OnDissolveFinished.AddUObject(this, &ThisClass::OnDissolveFinished);
}

void AETMonster::BeginPlay()
{
	Super::BeginPlay();

	if (GetCharacterType() == EETCharacterType::Boss)
	{
		StatBarWidgetComponent->DestroyComponent();
		StatBarWidgetComponent = nullptr;
	}
	// 위젯은 WidgetComponent::BeginPlay 에서 생성되고 첫 Tick 에 Construct 되므로 그 전에 소유 캐릭터 지정
	else if (UETStatListWidget* StatListWidget = Cast<UETStatListWidget>(StatBarWidgetComponent->GetUserWidgetObject()))
	{
		StatListWidget->SetOwningETCharacter(this);
		StatListWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

		// 최대값이 0 인 스탯(스턴 면역 등)은 빠지므로 표시되는 개수만큼 크기 지정
		StatBarWidgetComponent->SetDrawSize(StatListWidget->BuildStatWidgets(GetDefault<UETCommonSettings>()->UI.MonsterStatList));
	}

	UpdateStatBarVisible();
	LockOnWidgetComponent->SetVisibility(false);
}

void AETMonster::Die()
{
	// BT 공격 태스크가 중단되며 몽타주를 멈추므로 사망 몽타주 재생 전에 먼저 정지
	StopBrainLogic(TEXT("Death"));

	Super::Die();

	UpdateStatBarVisible();
	LockOnWidgetComponent->SetVisibility(false);

	if (AETSpawnVolume* SpawnVolume = GetHomeSpawnVolume())
	{
		SpawnVolume->OnSpawnedMonsterDied();
	}

	const FETCharacterTableRowData* CharacterRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETCharacterTableRowData>(CharacterRowName);
	if (CharacterRowData == nullptr)
	{
		return;
	}

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);
	if (PlayerController && CharacterRowData->DefeatData.NoticeWidgetRowName.IsNone() == false)
	{
		ETSubsystem::Get<UETJobSubsystem>(this).Run(PlayerController, MakeUnique<FETShowNoticeWidgetJob>(PlayerController, CharacterRowData->DefeatData.NoticeWidgetRowName));
	}
}

void AETMonster::Stun()
{
	// BT 액션 태스크가 중단되며 몽타주를 멈추고 이동도 계속되므로 스턴 몽타주 재생 전에 정지
	StopBrainLogic(TEXT("Stun"));

	Super::Stun();
}

void AETMonster::OnStunEnded(bool bInterrupted)
{
	Super::OnStunEnded(bInterrupted);

	if (IsDead())
	{
		return;
	}

	// 블랙보드는 유지되므로 인식 중이던 대상부터 다시 판단
	RestartBrainLogic();
}

bool AETMonster::PlayIntro()
{
	if (bIntroPlayed || GetActionMontageNum(ETGameplayTags::Character_Action_Intro) == 0)
	{
		return false;
	}

	bIntroPlayed = true;

	// BT 액션 태스크가 몽타주를 끊지 않도록 연출 동안 정지 (끝나면 블랙보드 대상으로 바로 전투)
	StopBrainLogic(TEXT("Intro"));

	if (const AActor* AttackTarget = GetAttackTarget())
	{
		SetActorRotation(FRotator(0.f, (AttackTarget->GetActorLocation() - GetActorLocation()).Rotation().Yaw, 0.f));
	}

	if (Super::PlayIntro() == false)
	{
		RestartBrainLogic();
		return false;
	}

	return true;
}

void AETMonster::OnIntroEnded(bool bInterrupted)
{
	Super::OnIntroEnded(bInterrupted);

	if (IsDead())
	{
		return;
	}

	RestartBrainLogic();
}

void AETMonster::Disappear()
{
	Destroy();
}

void AETMonster::OnDissolveFinished()
{
	Disappear();
}

float AETMonster::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	const float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	if (ActualDamage <= 0.f || IsDead())
	{
		return ActualDamage;
	}

	// 탐지 조건과 상관없이 공격한 대상 인식 (유지, 해제는 ET Find Target 에서 판정)
	DetectTarget(EventInstigator ? EventInstigator->GetPawn() : DamageCauser, false);

	return ActualDamage;
}

void AETMonster::DetectTarget(AActor* InTarget, const bool bInPlayIntro)
{
	UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (InTarget == nullptr || BlackboardComponent == nullptr || IsDead()
		|| BlackboardComponent->GetValueAsObject(ETNames::BlackboardKey::TargetActor) == InTarget)
	{
		return;
	}

	BlackboardComponent->SetValueAsObject(ETNames::BlackboardKey::TargetActor, InTarget);
	UpdateStatBarVisible();

	// 스턴, 피격 등 행동 중에는 연출로 끊지 않음 (연출 없이 바로 전투)
	const bool bActing = CharacterStateComponent->GetCurrentState().IsValid() && CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Idle) == false;
	if (bInPlayIntro && bActing == false)
	{
		PlayIntro();
	}

	if (AETSpawnVolume* SpawnVolume = GetHomeSpawnVolume())
	{
		SpawnVolume->AlertSpawnedMonsters(InTarget);
	}
}

void AETMonster::LoseTarget()
{
	if (UBlackboardComponent* BlackboardComponent = GetBlackboardComponent())
	{
		BlackboardComponent->SetValueAsObject(ETNames::BlackboardKey::TargetActor, nullptr);
	}

	UpdateStatBarVisible();
}

AActor* AETMonster::GetAttackTarget() const
{
	const UBlackboardComponent* BlackboardComponent = GetBlackboardComponent();
	if (BlackboardComponent == nullptr)
	{
		return nullptr;
	}

	return Cast<AActor>(BlackboardComponent->GetValueAsObject(ETNames::BlackboardKey::TargetActor));
}

AETSpawnVolume* AETMonster::GetHomeSpawnVolume() const
{
	return HomeSpawnVolume.Get();
}

void AETMonster::SetHomeSpawnVolume(AETSpawnVolume* InHomeSpawnVolume)
{
	HomeSpawnVolume = InHomeSpawnVolume;
}

void AETMonster::StopBrainLogic(const FString& InReason)
{
	const AAIController* AIController = Cast<AAIController>(GetController());
	if (UBrainComponent* BrainComponent = AIController ? AIController->GetBrainComponent() : nullptr)
	{
		BrainComponent->StopLogic(InReason);
	}
}

void AETMonster::RestartBrainLogic()
{
	const AAIController* AIController = Cast<AAIController>(GetController());
	if (UBrainComponent* BrainComponent = AIController ? AIController->GetBrainComponent() : nullptr)
	{
		BrainComponent->RestartLogic();
	}
}

UBlackboardComponent* AETMonster::GetBlackboardComponent() const
{
	AAIController* AIController = Cast<AAIController>(GetController());
	return AIController ? AIController->GetBlackboardComponent() : nullptr;
}

void AETMonster::UpdateStatBarVisible()
{
	if (StatBarWidgetComponent == nullptr)
	{
		return;
	}

	const bool bAlive = IsDead() == false;
	StatBarWidgetComponent->SetVisibility(bAlive && (GetAttackTarget() || bLockOnTargeted));
}

void AETMonster::OnLockOnTargeted()
{
	bLockOnTargeted = true;
	LockOnWidgetComponent->SetVisibility(true);
	UpdateStatBarVisible();
}

void AETMonster::OnLockOnTargetReleased()
{
	bLockOnTargeted = false;
	LockOnWidgetComponent->SetVisibility(false);
	UpdateStatBarVisible();
}
