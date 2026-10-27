#include "Interaction/ETLevelPortal.h"
#include "Character/ETPlayer.h"
#include "DataAsset/ETLevelTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Subsystem/ETLevelSubsystem.h"
#include "Components/BoxComponent.h"
#include "Particles/ParticleSystemComponent.h"

AETLevelPortal::AETLevelPortal()
{
	OpenParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("OpenParticleSystemComponent"));
	OpenParticleSystemComponent->SetupAttachment(GetRootComponent());
	OpenParticleSystemComponent->bAutoActivate = false;
}

void AETLevelPortal::BeginPlay()
{
	Super::BeginPlay();

	UETLevelSubsystem& LevelSubsystem = ETSubsystem::Get<UETLevelSubsystem>(this);
	CurrentLevelRowName = LevelSubsystem.FindLevelRowName(GetWorld());
	ensureMsgf(CurrentLevelRowName.IsNone() == false, TEXT("Level is not in level table. Portal: %s"), *GetName());

	OnBossDefeatedHandle = LevelSubsystem.OnBossDefeated.AddUObject(this, &ThisClass::OnBossDefeated);

	UpdatePortalOpen();
}

void AETLevelPortal::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UETLevelSubsystem* LevelSubsystem = ETSubsystem::GetPtr<UETLevelSubsystem>(this))
	{
		LevelSubsystem->OnBossDefeated.Remove(OnBossDefeatedHandle);
	}

	Super::EndPlay(EndPlayReason);
}

void AETLevelPortal::OnInteraction(AActor* InInteractor)
{
	Super::OnInteraction(InInteractor);

	if (bOpen == false || Cast<AETPlayer>(InInteractor) == nullptr)
	{
		return;
	}

	const FETLevelTableRowData* LevelRowData = ETSubsystem::Get<UETGameDataSubsystem>(this).GetRow<FETLevelTableRowData>(CurrentLevelRowName);
	if (LevelRowData == nullptr)
	{
		return;
	}

	UETLevelSubsystem& LevelSubsystem = ETSubsystem::Get<UETLevelSubsystem>(this);
	if (LevelRowData->NextLevelRowName.IsNone())
	{
		LevelSubsystem.OpenEndingLevel();
		return;
	}

	LevelSubsystem.OpenLevel(LevelRowData->NextLevelRowName);
}

void AETLevelPortal::OnBossDefeated(FName InCharacterRowName)
{
	UpdatePortalOpen();
}

void AETLevelPortal::UpdatePortalOpen()
{
	bOpen = CurrentLevelRowName.IsNone() == false && ETSubsystem::Get<UETLevelSubsystem>(this).IsLevelCleared(CurrentLevelRowName);

	// 닫혀 있으면 메시만 보이고 상호작용 박스에 걸리지 않도록
	InteractionBoxComponent->SetCollisionEnabled(bOpen ? ECollisionEnabled::QueryOnly : ECollisionEnabled::NoCollision);
	OpenParticleSystemComponent->SetActive(bOpen);
}
