#include "Spawn/ETSpawnVolume.h"
#include "Character/ETMonster.h"
#include "Character/ETPlayer.h"
#include "Components/ETCharacterStateComponent.h"
#include "Common/ETGameplayTags.h"
#include "DataAsset/ETCharacterTableRowData.h"
#include "DataAsset/ETSpawnVolumeTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Subsystem/ETSpawnSubsystem.h"
#include "Components/BillboardComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/PlayerController.h"
#include "Camera/PlayerCameraManager.h"
#include "Materials/MaterialInterface.h"
#include "NavigationSystem.h"
#include "UObject/ConstructorHelpers.h"

namespace
{
	// 순간이동 위치를 찾을 때 원 위에서 나눠 시도하는 횟수
	constexpr int32 TeleportAttemptCount = 8;
}

AETSpawnVolume::AETSpawnVolume()
{
	// 잠금 대상 플레이어가 있을 때만 Tick (잠금 중에는 플레이어 이동 후 위치 보정하도록 이동 컴포넌트를 선행 조건으로 추가)
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

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

	static ConstructorHelpers::FObjectFinder<UStaticMesh> SphereMesh(TEXT("/Engine/BasicShapes/Sphere.Sphere"));

	// 잠금은 수평 거리 기준이라 볼륨을 바닥 근처에 두면 플레이어 높이의 구 단면이 경계와 거의 같음 (아래 절반은 바닥에 가려짐)
	LockBoundaryMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LockBoundaryMeshComponent"));
	LockBoundaryMeshComponent->SetupAttachment(GetRootComponent());
	LockBoundaryMeshComponent->SetStaticMesh(SphereMesh.Object);
	LockBoundaryMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	LockBoundaryMeshComponent->SetCanEverAffectNavigation(false);
	LockBoundaryMeshComponent->SetCastShadow(false);
	LockBoundaryMeshComponent->SetVisibility(false);

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

	EndLock();

	RangeSphereComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnRangeSphereBeginOverlap);
	RangeSphereComponent->OnComponentEndOverlap.RemoveDynamic(this, &ThisClass::OnRangeSphereEndOverlap);

	// 셀 스트리밍으로 볼륨만 내려가도 몬스터가 남지 않도록 같이 제거 (다시 올라오면 BeginPlay 에서 재생성)
	if (EndPlayReason == EEndPlayReason::RemovedFromWorld)
	{
		DestroySpawnedCharacters();
	}

	Super::EndPlay(EndPlayReason);
}

void AETSpawnVolume::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AETPlayer* Player = LockTargetPlayer.Get();
	if (Player == nullptr)
	{
		EndLock();
		UpdateLockTickEnabled();
		return;
	}

	if (bPlayerLocked)
	{
		KeepPlayerInLockRange(Player);
		TryShowLockBoundary(Player);
		return;
	}

	// 범위 구체 오버랩은 캡슐이 닿는 순간이라 바로 잠그면 안쪽으로 끌려가므로 경계에서 진입 여유 거리만큼 더 들어온 뒤 잠금
	const FVector Offset = Player->GetActorLocation() - GetActorLocation();
	const float EnterDistance = FMath::Max(GetLockDistance(Player) - LockEnterMargin, 0.f);
	if (Offset.SizeSquared2D() <= FMath::Square(EnterDistance))
	{
		BeginLock(Player);
	}
}

void AETSpawnVolume::ResetSpawn()
{
	if (bRespawnOnReset == false)
	{
		return;
	}

	EndLock();
	DestroySpawnedCharacters();
	SpawnCharacters();
	UpdateLockTickEnabled();
}

bool AETSpawnVolume::IsInRange(const FVector& InLocation) const
{
	return FVector::Dist(GetActorLocation(), InLocation) <= RangeSphereComponent->GetScaledSphereRadius();
}

void AETSpawnVolume::OnRangeSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (IsPlayerCapsule(OtherActor, OtherComp))
	{
		bPlayerInRange = true;

		if (bLockPlayerInRange)
		{
			LockTargetPlayer = Cast<AETPlayer>(OtherActor);
			UpdateLockTickEnabled();
		}
	}
}

void AETSpawnVolume::OnRangeSphereEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (IsPlayerCapsule(OtherActor, OtherComp))
	{
		bPlayerInRange = false;

		// 잠금 중에는 높이 차이로 구체를 벗어나도 유지
		if (bPlayerLocked == false)
		{
			LockTargetPlayer.Reset();
			UpdateLockTickEnabled();
		}
	}
}

bool AETSpawnVolume::IsPlayerCapsule(const AActor* InOtherActor, const UPrimitiveComponent* InOtherComp) const
{
	const AETPlayer* Player = Cast<AETPlayer>(InOtherActor);
	return Player && InOtherComp == Player->GetCapsuleComponent();
}

void AETSpawnVolume::OnSpawnedMonsterDied()
{
	if (HasAliveSpawnedMonster())
	{
		return;
	}

	EndLock();
	UpdateLockTickEnabled();
}

void AETSpawnVolume::AlertSpawnedMonsters(AActor* InTarget)
{
	// 이미 같은 대상을 인식한 몬스터는 DetectTarget 에서 무시하므로 연쇄 호출이 반복되지 않음
	for (const TWeakObjectPtr<AETMonster>& SpawnedMonster : SpawnedMonsterArray)
	{
		if (AETMonster* Monster = SpawnedMonster.Get())
		{
			Monster->DetectTarget(InTarget, true);
		}
	}
}

bool AETSpawnVolume::IsTeleportDestination(const AETPlayer* InPlayer) const
{
	if (InPlayer == nullptr || IsInRange(InPlayer->GetActorLocation()) || HasAliveSpawnedMonster() == false)
	{
		return false;
	}

	bool bHasBoss = false;
	for (const TWeakObjectPtr<AETMonster>& SpawnedMonster : SpawnedMonsterArray)
	{
		if (SpawnedMonster.IsValid() && SpawnedMonster->GetCharacterType() == EETCharacterType::Boss)
		{
			bHasBoss = true;
			break;
		}
	}

	return bHasBoss == false;
}

bool AETSpawnVolume::FindTeleportTransform(const AETPlayer* InPlayer, OUT FTransform& OutTransform) const
{
	const UNavigationSystemV1* NavigationSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(GetWorld());
	if (InPlayer == nullptr || NavigationSystem == nullptr)
	{
		return false;
	}

	float CapsuleRadius = 0.f;
	float CapsuleHalfHeight = 0.f;
	InPlayer->GetCapsuleComponent()->GetScaledCapsuleSize(CapsuleRadius, CapsuleHalfHeight);

	// 잠금 볼륨이면 도착하자마자 잠기는 거리 (잠금이 없어도 경계 근처에서 몬스터를 바라보며 시작)
	const float Distance = FMath::Max(GetLockDistance(InPlayer) - LockEnterMargin, 0.f);
	const float StartAngle = FMath::FRandRange(0.f, UE_TWO_PI);
	const FVector Center = GetActorLocation();

	// 볼륨 높이와 지형 높이가 달라도 찾도록 위아래로 넉넉하게 투영
	const FVector ProjectExtent(CapsuleRadius, CapsuleRadius, 500.f);

	bool bFound = false;
	for (int32 Index = 0; Index < TeleportAttemptCount && bFound == false; ++Index)
	{
		const float Angle = StartAngle + UE_TWO_PI * Index / TeleportAttemptCount;
		const FVector CandidateLocation = Center + FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * Distance;

		FNavLocation NavLocation;
		if (NavigationSystem->ProjectPointToNavigation(CandidateLocation, NavLocation, ProjectExtent))
		{
			FVector TeleportLocation = NavLocation.Location + FVector(0.f, 0.f, CapsuleHalfHeight);
			const FRotator TeleportRotation(0.f, (Center - TeleportLocation).Rotation().Yaw, 0.f);

			// 벽, 지형에 캡슐이 겹치면 가까운 빈 위치로 보정 (보정할 수 없으면 다음 후보)
			if (GetWorld()->FindTeleportSpot(InPlayer, TeleportLocation, TeleportRotation))
			{
				OutTransform = FTransform(TeleportRotation, TeleportLocation);
				bFound = true;
			}
		}
	}

	return bFound;
}

bool AETSpawnVolume::HasAliveSpawnedMonster() const
{
	for (const TWeakObjectPtr<AETMonster>& SpawnedMonster : SpawnedMonsterArray)
	{
		if (SpawnedMonster.IsValid() && SpawnedMonster->IsDead() == false)
		{
			return true;
		}
	}

	return false;
}

void AETSpawnVolume::UpdateLockTickEnabled()
{
	SetActorTickEnabled(LockTargetPlayer.IsValid() && HasAliveSpawnedMonster());
}

float AETSpawnVolume::GetLockDistance(const AETPlayer* InPlayer) const
{
	return FMath::Max(RangeSphereComponent->GetScaledSphereRadius() - InPlayer->GetCapsuleComponent()->GetScaledCapsuleRadius(), 0.f);
}

void AETSpawnVolume::BeginLock(AETPlayer* InPlayer)
{
	bPlayerLocked = true;
	AddTickPrerequisiteComponent(InPlayer->GetCharacterMovement());
	TryShowLockBoundary(InPlayer);
}

void AETSpawnVolume::EndLock()
{
	if (bPlayerLocked == false)
	{
		return;
	}

	bPlayerLocked = false;
	LockBoundaryMeshComponent->SetVisibility(false);

	if (const AETPlayer* Player = LockTargetPlayer.Get())
	{
		RemoveTickPrerequisiteComponent(Player->GetCharacterMovement());
	}

	// 범위 밖에서 풀리면 리셋 후 다시 잠기지 않도록 대상 해제
	if (bPlayerInRange == false)
	{
		LockTargetPlayer.Reset();
	}
}

void AETSpawnVolume::KeepPlayerInLockRange(AETPlayer* InPlayer) const
{
	const FVector Center = GetActorLocation();
	const FVector PlayerLocation = InPlayer->GetActorLocation();
	const FVector Offset(PlayerLocation.X - Center.X, PlayerLocation.Y - Center.Y, 0.f);
	const float LockDistance = GetLockDistance(InPlayer);
	if (Offset.SizeSquared() <= FMath::Square(LockDistance))
	{
		return;
	}

	const FVector OutwardDirection = Offset.GetSafeNormal();
	InPlayer->SetActorLocation(FVector(Center.X, Center.Y, PlayerLocation.Z) + OutwardDirection * LockDistance);

	UCharacterMovementComponent* CharacterMovementComponent = InPlayer->GetCharacterMovement();
	const float OutwardSpeed = FVector::DotProduct(CharacterMovementComponent->Velocity, OutwardDirection);
	if (OutwardSpeed > 0.f)
	{
		CharacterMovementComponent->Velocity -= OutwardDirection * OutwardSpeed;
	}
}

void AETSpawnVolume::TryShowLockBoundary(const AETPlayer* InPlayer)
{
	if (LockBoundaryMaterial == nullptr || LockBoundaryMeshComponent->IsVisible())
	{
		return;
	}

	const APlayerController* PlayerController = Cast<APlayerController>(InPlayer->GetController());
	const APlayerCameraManager* PlayerCameraManager = PlayerController ? PlayerController->PlayerCameraManager : nullptr;
	if (PlayerCameraManager && IsInRange(PlayerCameraManager->GetCameraLocation()))
	{
		LockBoundaryMeshComponent->SetVisibility(true);
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

	bRespawnOnReset = SpawnVolumeRowData->bRespawnOnReset;
	bLockPlayerInRange = SpawnVolumeRowData->bLockPlayerInRange;
	LockEnterMargin = SpawnVolumeRowData->LockEnterMargin;
	LockBoundaryMaterial = bLockPlayerInRange ? SpawnVolumeRowData->LockBoundaryMaterial.LoadSynchronous() : nullptr;
	if (LockBoundaryMaterial && LockBoundaryMeshComponent->GetStaticMesh())
	{
		// 기본 구 메시 크기 기준으로 반지름을 Range 에 맞춤
		const FVector MeshExtent = LockBoundaryMeshComponent->GetStaticMesh()->GetBounds().BoxExtent;
		LockBoundaryMeshComponent->SetRelativeScale3D(FVector(SpawnVolumeRowData->Range / MeshExtent.X, SpawnVolumeRowData->Range / MeshExtent.Y, SpawnVolumeRowData->Range / MeshExtent.Z));
		LockBoundaryMeshComponent->SetMaterial(0, LockBoundaryMaterial);
	}

	for (const FETSpawnVolumeCharacterData& SpawnCharacterData : SpawnVolumeRowData->SpawnCharacterArray)
	{
		if (SpawnCharacterData.bSpawnEnabled == false)
		{
			continue;
		}

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
		Monster->SetHomeSpawnVolume(this);
		Monster->FinishSpawning(SpawnTransform);

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
		PreviewCapsuleComponent->ShapeColor = SpawnCharacterData.bSpawnEnabled ? FColor::Green : FColor::Silver;
		PreviewCapsuleComponent->SetupAttachment(GetRootComponent());
		PreviewCapsuleComponent->RegisterComponent();
		PreviewCapsuleComponent->SetWorldTransform(GetSpawnTransform(SpawnCharacterData, CapsuleHalfHeight));
	}
}
#endif
