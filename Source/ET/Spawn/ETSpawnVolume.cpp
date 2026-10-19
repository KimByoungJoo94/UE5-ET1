#include "Spawn/ETSpawnVolume.h"
#include "Character/ETMonster.h"
#include "Character/ETPlayer.h"
#include "DataAsset/ETCharacterTableRowData.h"
#include "DataAsset/ETSpawnVolumeTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Subsystem/ETSpawnSubsystem.h"
#include "Components/BillboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"

AETSpawnVolume::AETSpawnVolume()
{
	PrimaryActorTick.bCanEverTick = false;

	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootSceneComponent"));
	SetRootComponent(RootSceneComponent);

	RangeSphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("RangeSphereComponent"));
	RangeSphereComponent->SetupAttachment(GetRootComponent());
	// Pawn 오버랩만 받고 다른 트레이스는 막지 않도록 (OverlapOnlyPawn 프로필은 Visibility 등 나머지 채널이 Block)
	RangeSphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	RangeSphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	RangeSphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	RangeSphereComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	RangeSphereComponent->SetCanEverAffectNavigation(false);
	RangeSphereComponent->SetSphereRadius(0.f);

#if WITH_EDITORONLY_DATA
	// 거리와 상관없이 에디터에서 찾고 선택할 수 있도록 아이콘 표시
	SpriteComponent = CreateEditorOnlyDefaultSubobject<UBillboardComponent>(TEXT("SpriteComponent"));
	if (SpriteComponent)
	{
		SpriteComponent->SetupAttachment(GetRootComponent());
	}
#endif
}

void AETSpawnVolume::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

#if WITH_EDITOR
	// 게임 월드는 BeginPlay 에서 테이블 적용
	if (GetWorld() && GetWorld()->IsGameWorld() == false)
	{
		UpdateEditorPreview();
	}
#endif
}

void AETSpawnVolume::BeginPlay()
{
	Super::BeginPlay();

	if (UETSpawnSubsystem* SpawnSubsystem = ETSubsystem::GetPtr<UETSpawnSubsystem>(this))
	{
		SpawnSubsystem->RegisterSpawnVolume(this);
	}

	RangeSphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnRangeSphereBeginOverlap);
	RangeSphereComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnRangeSphereEndOverlap);

	SpawnCharacters();
}

void AETSpawnVolume::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (UETSpawnSubsystem* SpawnSubsystem = ETSubsystem::GetPtr<UETSpawnSubsystem>(this))
	{
		SpawnSubsystem->UnregisterSpawnVolume(this);
	}

	RangeSphereComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnRangeSphereBeginOverlap);
	RangeSphereComponent->OnComponentEndOverlap.RemoveDynamic(this, &ThisClass::OnRangeSphereEndOverlap);

	// 셀 스트리밍으로 볼륨만 내려가도 몬스터가 남지 않도록 같이 제거 (다시 올라오면 BeginPlay 에서 재생성)
	if (EndPlayReason == EEndPlayReason::RemovedFromWorld)
	{
		DestroySpawnedCharacters();
	}

	Super::EndPlay(EndPlayReason);
}

void AETSpawnVolume::ResetSpawn()
{
	DestroySpawnedCharacters();
	SpawnCharacters();
}

bool AETSpawnVolume::IsInRange(const FVector& InLocation) const
{
	return FVector::Dist(GetActorLocation(), InLocation) <= RangeSphereComponent->GetScaledSphereRadius();
}

void AETSpawnVolume::OnRangeSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsPlayerCapsule(OtherActor, OtherComp))
	{
		SetPlayerInRange(true);
	}
}

void AETSpawnVolume::OnRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsPlayerCapsule(OtherActor, OtherComp))
	{
		SetPlayerInRange(false);
	}
}

bool AETSpawnVolume::IsPlayerCapsule(const AActor* InOtherActor, const UPrimitiveComponent* InOtherComp) const
{
	const AETPlayer* Player = Cast<AETPlayer>(InOtherActor);
	return Player && InOtherComp == Player->GetCapsuleComponent();
}

void AETSpawnVolume::SetPlayerInRange(const bool bInRange)
{
	bPlayerInRange = bInRange;

	for (const TWeakObjectPtr<AETMonster>& SpawnedMonster : SpawnedMonsterArray)
	{
		if (SpawnedMonster.IsValid())
		{
			SpawnedMonster->SetStatBarVisible(bPlayerInRange);
		}
	}
}

void AETSpawnVolume::SpawnCharacters()
{
	const UETGameDataSubsystem& GameDataSubsystem = ETSubsystem::Get<UETGameDataSubsystem>(this);

	const FETSpawnVolumeTableRowData* SpawnVolumeRowData = GameDataSubsystem.GetRow<FETSpawnVolumeTableRowData>(SpawnVolumeRowName);
	if (ensureMsgf(SpawnVolumeRowData, TEXT("Invalid SpawnVolumeRowName. SpawnVolume: %s, RowName: %s"), *GetName(), *SpawnVolumeRowName.ToString()) == false)
	{
		return;
	}

	RangeSphereComponent->SetSphereRadius(SpawnVolumeRowData->Range);

	for (const FETSpawnVolumeCharacterData& SpawnCharacterData : SpawnVolumeRowData->SpawnCharacterArray)
	{
		const FETCharacterTableRowData* CharacterRowData = GameDataSubsystem.GetRow<FETCharacterTableRowData>(SpawnCharacterData.CharacterRowName);
		UClass* CharacterClass = CharacterRowData ? CharacterRowData->CharacterClass.LoadSynchronous() : nullptr;
		if (ensureMsgf(CharacterClass && CharacterClass->IsChildOf<AETMonster>(), TEXT("Invalid CharacterClass. SpawnVolume: %s, CharacterRowName: %s"), *GetName(), *SpawnCharacterData.CharacterRowName.ToString()) == false)
		{
			continue;
		}

		const float CapsuleHalfHeight = CharacterClass->GetDefaultObject<AETMonster>()->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
		const FTransform SpawnTransform = GetSpawnTransform(SpawnCharacterData, CapsuleHalfHeight);

		// PreInitializeComponents 에서 RowName 으로 데이터를 적용하므로 FinishSpawning 전에 지정
		AETMonster* Monster = GetWorld()->SpawnActorDeferred<AETMonster>(CharacterClass, SpawnTransform, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn);
		if (Monster == nullptr)
		{
			continue;
		}

		Monster->SetCharacterRowName(SpawnCharacterData.CharacterRowName);
		Monster->SetSpawnVolume(this);
		Monster->FinishSpawning(SpawnTransform);
		Monster->SetStatBarVisible(bPlayerInRange);

		SpawnedMonsterArray.Add(Monster);
	}
}

void AETSpawnVolume::DestroySpawnedCharacters()
{
	for (const TWeakObjectPtr<AETMonster>& SpawnedMonster : SpawnedMonsterArray)
	{
		if (SpawnedMonster.IsValid())
		{
			SpawnedMonster->Destroy();
		}
	}

	SpawnedMonsterArray.Reset();
}

FTransform AETSpawnVolume::GetSpawnTransform(const FETSpawnVolumeCharacterData& InSpawnCharacterData, const float InCapsuleHalfHeight) const
{
	// 볼륨 크기는 무시하고 위치, 회전만 적용
	const FTransform VolumeTransform(GetActorRotation(), GetActorLocation());
	const FTransform RelativeTransform(InSpawnCharacterData.RelativeRotation, InSpawnCharacterData.RelativeLocation);
	const FTransform SpawnTransform = RelativeTransform * VolumeTransform;

	return FTransform(FRotator(0.f, SpawnTransform.Rotator().Yaw, 0.f), FindGroundLocation(SpawnTransform.GetLocation(), InCapsuleHalfHeight));
}

FVector AETSpawnVolume::FindGroundLocation(const FVector& InLocation, const float InCapsuleHalfHeight) const
{
	// 볼륨 높이와 지형 높이가 달라도 바닥에 서도록 위아래로 탐색
	const FVector TraceOffset(0.f, 0.f, 500.f);

	FHitResult HitResult;
	const FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(ETSpawnVolumeGround), false, this);
	if (GetWorld()->LineTraceSingleByObjectType(HitResult, InLocation + TraceOffset, InLocation - TraceOffset, FCollisionObjectQueryParams(ECC_WorldStatic), QueryParams) == false)
	{
		return InLocation;
	}

	return HitResult.ImpactPoint + FVector(0.f, 0.f, InCapsuleHalfHeight);
}

#if WITH_EDITOR
void AETSpawnVolume::UpdateEditorPreview()
{
	const FETSpawnVolumeTableRowData* SpawnVolumeRowData = UETGameDataSubsystem::GetRowInEditor<FETSpawnVolumeTableRowData>(SpawnVolumeRowName);
	RangeSphereComponent->SetSphereRadius(SpawnVolumeRowData ? SpawnVolumeRowData->Range : 0.f);
	if (SpawnVolumeRowData == nullptr)
	{
		return;
	}

	for (const FETSpawnVolumeCharacterData& SpawnCharacterData : SpawnVolumeRowData->SpawnCharacterArray)
	{
		// 캐릭터 클래스가 비어 있어도 위치는 보이도록 기본 캡슐 크기 사용
		const FETCharacterTableRowData* CharacterRowData = UETGameDataSubsystem::GetRowInEditor<FETCharacterTableRowData>(SpawnCharacterData.CharacterRowName);
		const UClass* CharacterClass = CharacterRowData ? CharacterRowData->CharacterClass.LoadSynchronous() : nullptr;
		const AETCharacter* CharacterCDO = CharacterClass ? CharacterClass->GetDefaultObject<AETCharacter>() : nullptr;
		const float CapsuleRadius = CharacterCDO ? CharacterCDO->GetCapsuleComponent()->GetScaledCapsuleRadius() : 40.f;
		const float CapsuleHalfHeight = CharacterCDO ? CharacterCDO->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() : 90.f;

		// 생성 스크립트로 만든 컴포넌트로 지정해서 다시 실행될 때 이전 표시가 자동으로 제거되도록
		UCapsuleComponent* PreviewCapsuleComponent = NewObject<UCapsuleComponent>(this, NAME_None, RF_Transient);
		PreviewCapsuleComponent->CreationMethod = EComponentCreationMethod::UserConstructionScript;
		PreviewCapsuleComponent->bIsEditorOnly = true;
		PreviewCapsuleComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		PreviewCapsuleComponent->SetCanEverAffectNavigation(false);
		PreviewCapsuleComponent->SetCapsuleSize(CapsuleRadius, CapsuleHalfHeight);
		PreviewCapsuleComponent->ShapeColor = FColor::Green;
		PreviewCapsuleComponent->SetupAttachment(GetRootComponent());
		PreviewCapsuleComponent->RegisterComponent();
		PreviewCapsuleComponent->SetWorldTransform(GetSpawnTransform(SpawnCharacterData, CapsuleHalfHeight));
	}
}
#endif
