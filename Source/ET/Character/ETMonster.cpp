#include "Character/ETMonster.h"
#include "Components/WidgetComponent.h"
#include "UI/ETStatWidget.h"
#include "Common/ETCommonSettings.h"
#include "Controller/ETAIController.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "BehaviorTree/BlackboardComponent.h"

AETMonster::AETMonster()
{
	AIControllerClass = AETAIController::StaticClass();
	AutoPossessAI = EAutoPossessAI::PlacedInWorldOrSpawned;

	// 추적, 귀환은 이동 방향으로 회전 (공격 시 회전은 공격 태스크에서 처리)
	bUseControllerRotationYaw = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	HealthBarWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("HealthBarWidgetComponent"));
	HealthBarWidgetComponent->SetupAttachment(GetRootComponent());
	HealthBarWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 140.f));
	HealthBarWidgetComponent->SetDrawSize(FVector2D(130.f, 12.f));
	HealthBarWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);

	LockOnWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("LockOnWidgetComponent"));
	LockOnWidgetComponent->SetupAttachment(GetRootComponent());
	LockOnWidgetComponent->SetDrawSize(FVector2D(64.f, 64.f));
	LockOnWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 50.f));
	LockOnWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
	LockOnWidgetComponent->SetDrawAtDesiredSize(false);
}

void AETMonster::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	// 위젯 컴포넌트가 BeginPlay 에서 위젯을 생성하므로 그 전에 지정 (BP 에서 지정한 값이 우선)
	if (HealthBarWidgetComponent->GetWidgetClass() == nullptr)
	{
		HealthBarWidgetComponent->SetWidgetClass(GetDefault<UETCommonSettings>()->UI.HealthBarWidgetClass.LoadSynchronous());
	}

	if (LockOnWidgetComponent->GetWidgetClass() == nullptr)
	{
		LockOnWidgetComponent->SetWidgetClass(GetDefault<UETCommonSettings>()->UI.LockOnWidgetClass.LoadSynchronous());
	}
}

void AETMonster::BeginPlay()
{
	Super::BeginPlay();

	// 위젯은 WidgetComponent::BeginPlay 에서 생성되고 첫 Tick 에 Construct 되므로 그 전에 소유 캐릭터 지정
	if (UETStatWidget* HealthBarWidget = Cast<UETStatWidget>(HealthBarWidgetComponent->GetUserWidgetObject()))
	{
		HealthBarWidget->SetOwningETCharacter(this);
		HealthBarWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
		HealthBarWidget->SetUseStatText(false);

		// 소유 캐릭터 지정 후 바로 바인딩해서 첫 값 갱신 (Construct 시 다시 바인딩해도 기존 바인딩은 해제됨)
		HealthBarWidget->SetGameStatType(EETGameStatType::Health);
	}

	LockOnWidgetComponent->SetVisibility(false);
}

AActor* AETMonster::GetAttackTarget() const
{
	const AAIController* AIController = Cast<AAIController>(GetController());
	const UBlackboardComponent* BlackboardComponent = AIController ? AIController->GetBlackboardComponent() : nullptr;
	if (BlackboardComponent == nullptr)
	{
		return nullptr;
	}

	const FName TargetActorKeyName(TEXT("TargetActor"));
	return Cast<AActor>(BlackboardComponent->GetValueAsObject(TargetActorKeyName));
}

void AETMonster::OnLockOnTargeted()
{
	LockOnWidgetComponent->SetVisibility(true);
}

void AETMonster::OnLockOnTargetReleased()
{
	LockOnWidgetComponent->SetVisibility(false);
}
