#include "Interaction/ETFieldItem.h"
#include "Character/ETPlayer.h"
#include "Common/ETHelper.h"
#include "Components/ETAttackCollisionComponent.h"
#include "Components/ETInteractionComponent.h"
#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "DataAsset/ETFieldItemTableRowData.h"
#include "DataAsset/ETGimmickTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Spawn/ETSpawnVolume.h"
#include "Subsystem/ETSpawnSubsystem.h"
#include "TimerManager.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"

AETFieldItem::AETFieldItem()
{
	ParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("ParticleSystemComponent"));
	ParticleSystemComponent->SetupAttachment(GetRootComponent());

	// 기믹으로 생성한 몬스터가 상자 자리에서 바로 경로를 찾도록 내비메시에 구멍을 내지 않음
	StaticMeshComponent->SetCanEverAffectNavigation(false);
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

	const UETGameDataSubsystem& GameDataSubsystem = ETSubsystem::Get<UETGameDataSubsystem>(this);
	const FETFieldItemTableRowData* FieldItemRowData = GameDataSubsystem.GetRow<FETFieldItemTableRowData>(FieldItemRowName);
	if (FieldItemRowData)
	{
		Player->ApplyReward(FieldItemRowData->RewardRowName);
	}

	const FETGimmickTableRowData* GimmickRowData = FieldItemRowData ? GameDataSubsystem.GetRow<FETGimmickTableRowData>(FieldItemRowData->GimmickRowName) : nullptr;
	if (GimmickRowData)
	{
		// 남은 상호작용 동작 때문에 기믹에 대응하지 못하지 않도록 바로 조작을 돌려줌
		if (UETInteractionComponent* InteractionComponent = Player->GetInteractionComponent())
		{
			InteractionComponent->CancelInteraction();
		}

		StartGimmick(Player, GimmickRowData);
	}
	else
	{
		Destroy();
	}
}

void AETFieldItem::StartGimmick(AETCharacter* InInteractorCharacter, const FETGimmickTableRowData* InGimmickRowData)
{
	CachedGimmickRowData = InGimmickRowData;
	GimmickInstigatorCharacter = InInteractorCharacter;

	// 오버랩 종료로 상호작용 대상에서 해제되고 키 위젯도 숨겨짐
	InteractionBoxComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	// 유예가 없어도 상자가 사라진 뒤까지 보이도록 붙이지 않음
	const FVector WarningParticleLocation = GetGimmickCenter() + GetActorRotation().RotateVector(InGimmickRowData->WarningParticleLocationOffset);
	FETHelper::SpawnParticleAtLocation(this, InGimmickRowData->WarningParticle, WarningParticleLocation, GetActorRotation(), InGimmickRowData->WarningParticleScale);

	if (InGimmickRowData->Delay > 0.f)
	{
		GetWorldTimerManager().SetTimer(GimmickTimerHandle, this, &ThisClass::TriggerGimmick, InGimmickRowData->Delay, false);
	}
	else
	{
		TriggerGimmick();
	}
}

void AETFieldItem::TriggerGimmick()
{
	if (CachedGimmickRowData)
	{
		AETCharacter* InstigatorCharacter = GimmickInstigatorCharacter.Get();

		// 생성한 몬스터가 상자 충돌에 밀려나지 않도록 먼저 끔
		SetActorEnableCollision(false);

		if (CachedGimmickRowData->ExplosionRadialAttackCollisionRowName.IsNone() == false)
		{
			UETAttackCollisionComponent::PlayRadialExplosion(this, InstigatorCharacter, CachedGimmickRowData->ExplosionRadialAttackCollisionRowName, GetGimmickCenter(), CachedGimmickRowData->ExplosionDamage);
		}

		if (CachedGimmickRowData->SpawnVolumeRowName.IsNone() == false)
		{
			// BeginPlay 에서 테이블 행으로 몬스터를 생성하므로 FinishSpawning 전에 지정
			const FTransform SpawnTransform(FRotator(0.f, GetActorRotation().Yaw, 0.f), GetActorLocation());
			if (AETSpawnVolume* SpawnVolume = GetWorld()->SpawnActorDeferred<AETSpawnVolume>(AETSpawnVolume::StaticClass(), SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn))
			{
				SpawnVolume->SetSpawnVolumeRowName(CachedGimmickRowData->SpawnVolumeRowName);
				SpawnVolume->FinishSpawning(SpawnTransform);

				if (InstigatorCharacter)
				{
					SpawnVolume->AlertSpawnedMonsters(InstigatorCharacter);
				}
			}
		}

		if (CachedGimmickRowData->bTeleport)
		{
			if (AETPlayer* Player = Cast<AETPlayer>(InstigatorCharacter))
			{
				FTransform TeleportTransform;
				if (AETSpawnVolume* DestinationVolume = ETSubsystem::Get<UETSpawnSubsystem>(this).FindTeleportDestination(Player, TeleportTransform))
				{
					Player->TeleportByGimmick(TeleportTransform);
					DestinationVolume->AlertSpawnedMonsters(Player);
				}
			}
		}
	}

	Destroy();
}

FVector AETFieldItem::GetGimmickCenter() const
{
	return StaticMeshComponent->GetStaticMesh() ? StaticMeshComponent->Bounds.Origin : GetActorLocation();
}

void AETFieldItem::ApplyFieldItemRowData(const FETFieldItemTableRowData* InFieldItemRowData)
{
	if (InFieldItemRowData == nullptr || InFieldItemRowData->ParticleSystem.IsNull())
	{
		return;
	}

	ParticleSystemComponent->SetTemplate(InFieldItemRowData->ParticleSystem.LoadSynchronous());
}
