#include "Interaction/ETInteractionActor.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/ETInteractionComponent.h"
#include "Character/ETPlayer.h"
#include "UI/ETInteractionWidget.h"
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
	InteractionWidgetComponent->SetDrawSize(FVector2D(50.f, 50.f));
	InteractionWidgetComponent->SetWidgetSpace(EWidgetSpace::Screen);
}

void AETInteractionActor::BeginPlay()
{
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
		GEngine->AddOnScreenDebugMessage(-1, 3.f, FColor::Yellow, FString::Printf(TEXT("%s::OnInteraction"), *GetClass()->GetName()));
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
