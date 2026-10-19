#include "Interaction/ETCheckpoint.h"
#include "Character/ETPlayer.h"
#include "Controller/ETPlayerController.h"
#include "Subsystem/ETSaveSubsystem.h"

AETCheckpoint::AETCheckpoint()
{
	RespawnPointComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RespawnPointComponent"));
	RespawnPointComponent->SetupAttachment(GetRootComponent());
	RespawnPointComponent->SetRelativeLocation(FVector(300.f, 0.f, 100.f));

#if WITH_EDITORONLY_DATA
	// 플레이어 스폰 시점(셀 스트리밍 전)에 찾을 수 있도록 상시 로드 (PlayerStart 와 동일)
	bIsSpatiallyLoaded = false;
#endif
}

void AETCheckpoint::OnInteraction(AActor* InInteractor)
{
	Super::OnInteraction(InInteractor);

	AETPlayer* Player = Cast<AETPlayer>(InInteractor);
	if (Player == nullptr)
	{
		return;
	}

	// 발견 기록, 부활 위치 갱신 (회복, 몬스터 리스폰, 이동은 체크포인트 창 버튼으로 처리)
	UETSaveSubsystem& SaveSubsystem = ETSubsystem::Get<UETSaveSubsystem>(this);
	FETWorldSaveData& WorldSaveData = SaveSubsystem.GetWorldSaveData();
	WorldSaveData.DiscoveredCheckpointRowNameSet.Add(CheckpointRowName);
	WorldSaveData.CheckpointRowName = CheckpointRowName;
	SaveSubsystem.Save();

	if (AETPlayerController* PlayerController = Cast<AETPlayerController>(Player->GetController()))
	{
		PlayerController->OpenCheckpointWidget(CheckpointRowName);
	}
}

FTransform AETCheckpoint::GetRespawnTransform() const
{
	// 부활 위치를 옮겨도 항상 체크포인트를 바라보도록 방향 계산
	const FVector RespawnLocation = RespawnPointComponent->GetComponentLocation();
	const FVector LookDirection = GetActorLocation() - RespawnLocation;

	return FTransform(FRotator(0.f, LookDirection.Rotation().Yaw, 0.f), RespawnLocation);
}
