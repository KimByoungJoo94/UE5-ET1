#pragma once

#include "CoreMinimal.h"
#include "Interaction/ETInteractionActor.h"
#include "ETLevelPortal.generated.h"

class UParticleSystemComponent;

// 현재 레벨이 클리어되면 열리고 다음 레벨로 이동, 다음 레벨이 없으면 엔딩 레벨로 이동 (레벨에 미리 배치, 열리면 파티클 재생 및 상호작용 가능)
UCLASS()
class ET_API AETLevelPortal : public AETInteractionActor
{
	GENERATED_BODY()

public:
	AETLevelPortal();

protected:
	virtual void BeginPlay() override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

public:
	// IETInteractionInterface
	virtual void OnInteraction(AActor* InInteractor) override;
	// ~IETInteractionInterface

protected:
	void OnLevelCleared(FName InLevelRowName);

private:
	void UpdatePortalOpen();

protected:	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UParticleSystemComponent> OpenParticleSystemComponent;

private:	
	FName CurrentLevelRowName;

	bool bOpen = false;

	FDelegateHandle OnLevelClearedHandle;
};
