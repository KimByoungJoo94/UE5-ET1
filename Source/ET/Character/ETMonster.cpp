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
#include "Subsystem/ETLevelSubsystem.h"
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
	LockOnWidgetComponent->SetupAttachment(GetRootComponent());
	LockOnWidgetComponent->SetDrawSize(FVector2D(40.f, 40.f));
	LockOnWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
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

	// 위젯은 WidgetComponent::BeginPlay 에서 생성되고 첫 Tick 에 Construct 되므로 그 전에 소유 캐릭터 지정
	if (UETStatListWidget* StatListWidget = Cast<UETStatListWidget>(StatBarWidgetComponent->GetUserWidgetObject()))
	{
		StatListWidget->SetOwningETCharacter(this);
		StatListWidget->SetVisibility(ESlateVisibility::HitTestInvisible);

		// 최대값이 0 인 스탯(스턴 면역 등)은 빠지므로 표시되는 개수만큼 크기 지정
		StatBarWidgetComponent->SetDrawSize(StatListWidget->BuildStatWidgets(GetDefault<UETCommonSettings>()->UI.MonsterStatList));
	}

	// 플레이어가 스폰 볼륨에 들어왔을 때만 표시
	StatBarWidgetComponent->SetVisibility(false);
	LockOnWidgetComponent->SetVisibility(false);
}

void AETMonster::Die()
{
	// BT 공격 태스크가 중단되며 몽타주를 멈추므로 사망 몽타주 재생 전에 먼저 정지
	if (const AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* BrainComponent = AIController->GetBrainComponent())
		{
			BrainComponent->StopLogic(TEXT("Death"));
		}
	}

	Super::Die();

	StatBarWidgetComponent->SetVisibility(false);
	LockOnWidgetComponent->SetVisibility(false);

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

	if (CharacterRowData->CharacterType == EETCharacterType::Boss)
	{
		ETSubsystem::Get<UETLevelSubsystem>(this).RecordBossDefeat(CharacterRowName);
	}
}

void AETMonster::Stun()
{
	// BT 액션 태스크가 중단되며 몽타주를 멈추고 이동도 계속되므로 스턴 몽타주 재생 전에 정지
	if (const AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* BrainComponent = AIController->GetBrainComponent())
		{
			BrainComponent->StopLogic(TEXT("Stun"));
		}
	}

	Super::Stun();
}

void AETMonster::OnStunEnded(bool bInterrupted)
{
	Super::OnStunEnded(bInterrupted);

	if (CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Death))
	{
		return;
	}

	// 블랙보드는 유지되므로 인식 중이던 대상부터 다시 판단
	if (const AAIController* AIController = Cast<AAIController>(GetController()))
	{
		if (UBrainComponent* BrainComponent = AIController->GetBrainComponent())
		{
			BrainComponent->RestartLogic();
		}
	}
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
	if (ActualDamage <= 0.f || CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Death))
	{
		return ActualDamage;
	}

	// 탐지 조건과 상관없이 공격한 대상 인식 (유지, 해제는 ET Find Target 에서 판정)
	AAIController* AIController = Cast<AAIController>(GetController());
	if (UBlackboardComponent* BlackboardComponent = AIController ? AIController->GetBlackboardComponent() : nullptr)
	{
		AActor* Attacker = EventInstigator ? EventInstigator->GetPawn() : DamageCauser;
		BlackboardComponent->SetValueAsObject(ETNames::BlackboardKey::TargetActor, Attacker);
	}

	return ActualDamage;
}

AActor* AETMonster::GetAttackTarget() const
{
	const AAIController* AIController = Cast<AAIController>(GetController());
	const UBlackboardComponent* BlackboardComponent = AIController ? AIController->GetBlackboardComponent() : nullptr;
	if (BlackboardComponent == nullptr)
	{
		return nullptr;
	}

	return Cast<AActor>(BlackboardComponent->GetValueAsObject(ETNames::BlackboardKey::TargetActor));
}

AETSpawnVolume* AETMonster::GetSpawnVolume() const
{
	return SpawnVolume.Get();
}

void AETMonster::SetSpawnVolume(AETSpawnVolume* InSpawnVolume)
{
	SpawnVolume = InSpawnVolume;
}

void AETMonster::SetStatBarVisible(const bool bVisible)
{
	StatBarWidgetComponent->SetVisibility(bVisible && CharacterStateComponent->IsCurrentState(ETGameplayTags::Character_State_Death) == false);
}

void AETMonster::OnLockOnTargeted()
{
	LockOnWidgetComponent->SetVisibility(true);
}

void AETMonster::OnLockOnTargetReleased()
{
	LockOnWidgetComponent->SetVisibility(false);
}
