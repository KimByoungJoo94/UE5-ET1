#include "Item/ETDropItem.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/WidgetComponent.h"
#include "Components/ETInteractionComponent.h"
#include "Character/ETPlayer.h"
#include "UI/ETInteractionWidget.h"

AETDropItem::AETDropItem()
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

void AETDropItem::BeginPlay()
{
	Super::BeginPlay();	
	
	InteractionWidgetComponent->SetVisibility(false);
	
	InteractionBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnInteractionBoxBeginOverlap);
	InteractionBoxComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnInteractionBoxEndOverlap);
}

void AETDropItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	InteractionBoxComponent->OnComponentBeginOverlap.RemoveDynamic(this, &AETDropItem::OnInteractionBoxBeginOverlap);
	InteractionBoxComponent->OnComponentEndOverlap.RemoveDynamic(this, &AETDropItem::OnInteractionBoxEndOverlap);

	Super::EndPlay(EndPlayReason);
}

void AETDropItem::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AETDropItem::OnInteractionBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
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

void AETDropItem::OnInteractionBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AETPlayer* Player = Cast<AETPlayer>(OtherActor))
	{
		if (UETInteractionComponent* InteractionComponent = Player->GetInteractionComponent())
		{
			InteractionComponent->ClearInteractionTarget();
			InteractionWidgetComponent->SetVisibility(false);			
		}
	}
}

void AETDropItem::OnInteraction(AActor* InInteractor)
{
	UE_LOG(LogTemp, Log, TEXT("AETDropItem::OnInteraction(AActor* InInteractor)"));

	// TODO
}


