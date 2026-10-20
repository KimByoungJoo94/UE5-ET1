#include "Interaction/ETFieldItem.h"
#include "Character/ETPlayer.h"
#include "DataAsset/ETFieldItemTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"

AETFieldItem::AETFieldItem()
{
	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->SetupAttachment(GetRootComponent());
}

void AETFieldItem::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	// 게임 월드는 PostInitializeComponents 에서 테이블 적용
	if (GetWorld() && GetWorld()->IsGameWorld() == false)
	{
		ApplyFieldItemRowData(UETGameDataSubsystem::GetRowInEditor<FETFieldItemTableRowData>(FieldItemRowName));
	}
#endif
}

void AETFieldItem::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GetWorld() && GetWorld()->IsGameWorld())
	{
		if (const UETGameDataSubsystem* GameDataSubsystem = ETSubsystem::GetPtr<UETGameDataSubsystem>(this))
		{
			ApplyFieldItemRowData(GameDataSubsystem->GetRow<FETFieldItemTableRowData>(FieldItemRowName));
		}
	}
}

void AETFieldItem::OnInteraction(AActor* InInteractor)
{
	Super::OnInteraction(InInteractor);

	AETPlayer* Player = Cast<AETPlayer>(InInteractor);
	if (Player == nullptr)
	{
		return;
	}

	const FETFieldItemTableRowData* FieldItemRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETFieldItemTableRowData>(FieldItemRowName);
	if (FieldItemRowData)
	{
		Player->ApplyReward(FieldItemRowData->RewardRowName);
	}

	Destroy();
}

void AETFieldItem::ApplyFieldItemRowData(const FETFieldItemTableRowData* InFieldItemRowData)
{
	if (InFieldItemRowData == nullptr || InFieldItemRowData->ParticleSystem.IsNull())
	{
		return;
	}

	ParticleSystemComponent->SetTemplate(InFieldItemRowData->ParticleSystem.LoadSynchronous());
}
