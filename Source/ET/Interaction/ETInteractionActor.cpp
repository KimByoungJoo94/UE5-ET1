#include "Interaction/ETInteractionActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/ArrowComponent.h"
#include "Components/ETInteractionComponent.h"
#include "Character/ETPlayer.h"
#include "UI/ETInteractionWidget.h"
#include "Common/ETCommonSettings.h"
#include "Engine/Engine.h"

AETInteractionActor::AETInteractionActor()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	StaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
	StaticMeshComponent->SetupAttachment(GetRootComponent());

	InteractionBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionBoxComponent"));
	InteractionBoxComponent->SetupAttachment(GetRootComponent());
	InteractionBoxComponent->SetGenerateOverlapEvents(true);

	InteractionWidgetComponent = CreateDefaultSubobject<UWidgetComponent>(TEXT("InteractionWidgetComponent"));
	InteractionWidgetComponent->SetupAttachment(GetRootComponent());
	InteractionWidgetComponent->SetRelativeLocation(FVector(0.f, 0.f, 200.f));
	InteractionWidgetComponent->SetDrawSize(FVector2D(50.f, 50.f));
	InteractionWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);

#if WITH_EDITORONLY_DATA
	ArrowComponent = CreateEditorOnlyDefaultSubobject<UArrowComponent>(TEXT("ArrowComponent"));
	if (ArrowComponent)
	{
		ArrowComponent->SetupAttachment(GetRootComponent());
	}
#endif
}

void AETInteractionActor::BeginPlay()
{
	// 위젯 컴포넌트가 BeginPlay 에서 위젯을 생성하므로 Super 전에 설정 (BP 에서 지정한 위젯 우선)
	if (InteractionWidgetComponent->GetWidgetClass() == nullptr)
	{
		InteractionWidgetComponent->SetWidgetClass(GetDefault<UETCommonSettings>()->UI.InteractionWidgetClass.LoadSynchronous());
	}

	Super::BeginPlay();

	InteractionWidgetComponent->SetVisibility(false);

	InteractionBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnInteractionBoxBeginOverlap);
	InteractionBoxComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnInteractionBoxEndOverlap);
}

void AETInteractionActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	InteractionBoxComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnInteractionBoxBeginOverlap);
	InteractionBoxComponent->OnComponentEndOverlap.RemoveDynamic(this, &ThisClass::OnInteractionBoxEndOverlap);

	Super::EndPlay(EndPlayReason);
}

void AETInteractionActor::OnInteraction(AActor* InInteractor)
{
#if !UE_BUILD_SHIPPING
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Black, FString::Printf(TEXT("%s::OnInteraction"), *GetClass()->GetName()));
	}
#endif
}

void AETInteractionActor::OnInteractionTargetReleased()
{
	InteractionWidgetComponent->SetVisibility(false);
}

void AETInteractionActor::OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AETPlayer* Player = Cast<AETPlayer>(OtherActor))
	{
		if (UETInteractionComponent* InteractionComponent = Player->GetInteractionComponent())
		{
			InteractionComponent->SetInteractionTarget(this);

			UETInteractionWidget* InteractionWidget = Cast<UETInteractionWidget>(InteractionWidgetComponent->GetUserWidgetObject());
			if (IsValid(InteractionWidget))
			{
				InteractionWidget->UpdateKeyText();
			}

			InteractionWidgetComponent->SetVisibility(true);
		}
	}
}

void AETInteractionActor::OnInteractionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AETPlayer* Player = Cast<AETPlayer>(OtherActor))
	{
		if (UETInteractionComponent* InteractionComponent = Player->GetInteractionComponent())
		{
			// 다른 상호작용 액터가 대상으로 등록된 경우 유지 (위젯은 대상 교체 시 이미 숨김)
			if (InteractionComponent->GetInteractionTarget() == this)
			{
				InteractionComponent->ClearInteractionTarget();
			}
		}
	}
}
