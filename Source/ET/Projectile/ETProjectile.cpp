#include "Projectile/ETProjectile.h"
#include "Character/ETCharacter.h"
#include "Common/ETHelper.h"
#include "DataAsset/ETCharacterTableRowData.h"
#include "DataAsset/ETProjectileTableRowData.h"
#include "Subsystem/ETGameDataSubsystem.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystem.h"
#include "Particles/ParticleSystemComponent.h"

AETProjectile::AETProjectile()
	: OwnerCharacterType(EETCharacterType::Monster)
{
	PrimaryActorTick.bCanEverTick = false;

	// 지형은 막혀서 멈추고, 테이블 대상 채널은 겹침으로 판정 (PreInitializeComponents 에서 추가)
	SphereComponent = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComponent"));
	SphereComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	SphereComponent->SetCollisionObjectType(ECC_WorldDynamic);
	SphereComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	SphereComponent->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	SphereComponent->SetGenerateOverlapEvents(true);
	SphereComponent->SetCanEverAffectNavigation(false);
	SetRootComponent(SphereComponent);

	RootParticleSystemComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("RootParticleSystemComponent"));
	RootParticleSystemComponent->SetupAttachment(GetRootComponent());
	RootParticleSystemComponent->bAutoActivate = false;

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;
	ProjectileMovementComponent->bShouldBounce = false;
	ProjectileMovementComponent->ProjectileGravityScale = 0.f;
}

void AETProjectile::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	const UETGameDataSubsystem* GameDataSubsystem = ETSubsystem::GetPtr<UETGameDataSubsystem>(this);
	if (GameDataSubsystem == nullptr)
	{
		return;
	}

	CachedProjectileRowData = GameDataSubsystem->GetRow<FETProjectileTableRowData>(ProjectileRowName);
	if (ensureMsgf(CachedProjectileRowData, TEXT("Invalid ProjectileRowName. Projectile: %s, RowName: %s"), *GetName(), *ProjectileRowName.ToString()) == false)
	{
		return;
	}

	SphereComponent->SetSphereRadius(CachedProjectileRowData->Radius);
	for (const TEnumAsByte<EObjectTypeQuery>& ObjectType : CachedProjectileRowData->ObjectTypeArray)
	{
		SphereComponent->SetCollisionResponseToChannel(UEngineTypes::ConvertToCollisionChannel(ObjectType), ECR_Overlap);
	}

	// 이동 컴포넌트가 InitializeComponent 에서 초기 속도를 정하므로 그 전에 지정 (방향은 액터 정면)
	ProjectileMovementComponent->InitialSpeed = CachedProjectileRowData->Speed;
	ProjectileMovementComponent->MaxSpeed = CachedProjectileRowData->Speed;
	ProjectileMovementComponent->ProjectileGravityScale = CachedProjectileRowData->MoveType == EETProjectileMoveType::Parabolic ? CachedProjectileRowData->GravityScale : 0.f;

	OwnerCharacter = Cast<AETCharacter>(GetInstigator());
	if (AETCharacter* Character = OwnerCharacter.Get())
	{
		OwnerCharacterType = Character->GetCharacterType();
		AttackDamage = Character->GetAttackDamage();
		SphereComponent->IgnoreActorWhenMoving(Character, true);
	}
}

void AETProjectile::BeginPlay()
{
	Super::BeginPlay();

	SphereComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnSphereBeginOverlap);
	ProjectileMovementComponent->OnProjectileStop.AddDynamic(this, &ThisClass::OnProjectileStop);

	if (CachedProjectileRowData == nullptr)
	{
		Destroy();
		return;
	}

	if (CachedProjectileRowData->MaxLifeTime > 0.f)
	{
		SetLifeSpan(CachedProjectileRowData->MaxLifeTime);
	}

	if (UParticleSystem* RootParticle = CachedProjectileRowData->RootParticle.LoadSynchronous())
	{
		RootParticleSystemComponent->SetTemplate(RootParticle);
		RootParticleSystemComponent->SetRelativeScale3D(FVector(CachedProjectileRowData->RootParticleScale));
		RootParticleSystemComponent->Activate(true);
	}
}

void AETProjectile::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	SphereComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnSphereBeginOverlap);
	ProjectileMovementComponent->OnProjectileStop.RemoveDynamic(this, &ThisClass::OnProjectileStop);

	Super::EndPlay(EndPlayReason);
}

void AETProjectile::OnSphereBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// 캐릭터만 충돌 대상 (발사자 제외, 같은 편도 충돌 후 소멸하고 데미지만 없음)
	AETCharacter* TargetCharacter = Cast<AETCharacter>(OtherActor);
	if (bHit || TargetCharacter == nullptr || TargetCharacter == GetInstigator())
	{
		return;
	}

	// 생성 위치에서 이미 겹친 경우 등 스윕 정보가 없으면 현재 위치 기준
	const FHitResult HitResult = bFromSweep ? SweepResult : FHitResult(OtherActor, OtherComp, GetActorLocation(), -GetVelocity().GetSafeNormal());
	ProcessHit(HitResult, TargetCharacter);
}

void AETProjectile::OnProjectileStop(const FHitResult& ImpactResult)
{
	if (bHit)
	{
		return;
	}

	ProcessHit(ImpactResult, nullptr);
}

void AETProjectile::ProcessHit(const FHitResult& InHitResult, AETCharacter* InTargetCharacter)
{
	bHit = true;

	float ActualDamage = 0.f;
	if (InTargetCharacter)
	{
		if (AETCharacter* Character = OwnerCharacter.Get())
		{
			ActualDamage = Character->AttackTarget(InTargetCharacter, InHitResult, AttackDamage);
		}
		else if (FETHelper::IsHostile(OwnerCharacterType, InTargetCharacter->GetCharacterType()))
		{
			ActualDamage = UGameplayStatics::ApplyPointDamage(InTargetCharacter, AttackDamage, GetVelocity().GetSafeNormal(), InHitResult, nullptr, this, UDamageType::StaticClass());
		}
	}

	if (CachedProjectileRowData && (InTargetCharacter == nullptr || ActualDamage > 0.f))
	{
		FETHelper::SpawnParticleAtLocation(this, CachedProjectileRowData->HitParticle, InHitResult.ImpactPoint, CachedProjectileRowData->HitParticleScale);
	}

	Destroy();
}
