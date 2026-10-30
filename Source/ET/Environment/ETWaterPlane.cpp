#include "Environment/ETWaterPlane.h"
#include "Character/ETCharacter.h"
#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"

AETWaterPlane::AETWaterPlane()
{
	// 판정 영역에 캐릭터가 있을 때만 Tick
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = false;

	WaterMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("WaterMeshComponent"));
	SetRootComponent(WaterMeshComponent);
	WaterMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	WaterMeshComponent->SetCanEverAffectNavigation(false);

	// 루트 스케일과 상관없이 월드 크기로 맞추도록 절대 스케일
	WaterBoxComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("WaterBoxComponent"));
	WaterBoxComponent->SetupAttachment(GetRootComponent());
	WaterBoxComponent->SetUsingAbsoluteScale(true);
	WaterBoxComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	WaterBoxComponent->SetCollisionObjectType(ECC_WorldDynamic);
	WaterBoxComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
	WaterBoxComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	WaterBoxComponent->SetGenerateOverlapEvents(true);
	WaterBoxComponent->SetCanEverAffectNavigation(false);
}

void AETWaterPlane::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);

	UpdateWaterBox();
}

void AETWaterPlane::BeginPlay()
{
	Super::BeginPlay();

	UpdateWaterBox();

	WaterBoxComponent->OnComponentBeginOverlap.AddDynamic(this, &ThisClass::OnWaterBoxBeginOverlap);
	WaterBoxComponent->OnComponentEndOverlap.AddDynamic(this, &ThisClass::OnWaterBoxEndOverlap);
}

void AETWaterPlane::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	WaterBoxComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ThisClass::OnWaterBoxBeginOverlap);
	WaterBoxComponent->OnComponentEndOverlap.RemoveDynamic(this, &ThisClass::OnWaterBoxEndOverlap);

	Super::EndPlay(EndPlayReason);
}

void AETWaterPlane::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	const float SurfaceHeight = GetSurfaceHeight();

	// 사망하면 충돌이 꺼지며 오버랩 종료로 집합에서 빠지므로 복사본으로 순회
	const TArray<TWeakObjectPtr<AETCharacter>> InWaterCharacterArray = InWaterCharacterSet.Array();
	for (const TWeakObjectPtr<AETCharacter>& InWaterCharacter : InWaterCharacterArray)
	{
		AETCharacter* Character = InWaterCharacter.Get();
		if (Character == nullptr || Character->IsDead())
		{
			InWaterCharacterSet.Remove(InWaterCharacter);
		}
		else
		{
			// 캡슐 꼭대기까지 수면 아래면 몸 전체가 잠긴 것으로 판단
			const float CapsuleTopHeight = Character->GetActorLocation().Z + Character->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
			if (CapsuleTopHeight < SurfaceHeight)
			{
				Character->Kill();
			}
		}
	}

	SetActorTickEnabled(InWaterCharacterSet.IsEmpty() == false);
}

void AETWaterPlane::OnWaterBoxBeginOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (AETCharacter* Character = GetCharacterByCapsule(OtherActor, OtherComp))
	{
		InWaterCharacterSet.Add(Character);
		SetActorTickEnabled(true);
	}
}

void AETWaterPlane::OnWaterBoxEndOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (AETCharacter* Character = GetCharacterByCapsule(OtherActor, OtherComp))
	{
		InWaterCharacterSet.Remove(Character);
		SetActorTickEnabled(InWaterCharacterSet.IsEmpty() == false);
	}
}

void AETWaterPlane::UpdateWaterBox()
{
	if (WaterMeshComponent->GetStaticMesh() == nullptr)
	{
		return;
	}

	// 메시 바운드는 월드 기준이므로 박스는 절대 스케일로 월드 크기 그대로 지정
	const FBoxSphereBounds& MeshBounds = WaterMeshComponent->Bounds;
	const float SurfaceHeight = GetSurfaceHeight();

	WaterBoxComponent->SetBoxExtent(FVector(MeshBounds.BoxExtent.X, MeshBounds.BoxExtent.Y, WaterDepth * 0.5f));
	WaterBoxComponent->SetWorldLocationAndRotation(FVector(MeshBounds.Origin.X, MeshBounds.Origin.Y, SurfaceHeight - WaterDepth * 0.5f), FRotator::ZeroRotator);
}

float AETWaterPlane::GetSurfaceHeight() const
{
	return WaterMeshComponent->GetComponentLocation().Z;
}

bool AETWaterPlane::IsInWaterArea(const FVector& InLocation) const
{
	const FVector BoxCenter = WaterBoxComponent->GetComponentLocation();
	const FVector BoxExtent = WaterBoxComponent->GetScaledBoxExtent();
	return FMath::Abs(InLocation.X - BoxCenter.X) <= BoxExtent.X && FMath::Abs(InLocation.Y - BoxCenter.Y) <= BoxExtent.Y;
}

AETCharacter* AETWaterPlane::GetCharacterByCapsule(AActor* InOtherActor, const UPrimitiveComponent* InOtherComp) const
{
	AETCharacter* Character = Cast<AETCharacter>(InOtherActor);
	return Character && Character->GetCapsuleComponent() == InOtherComp ? Character : nullptr;
}
