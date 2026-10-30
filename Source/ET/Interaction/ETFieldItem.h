#pragma once

#include "CoreMinimal.h"
#include "Interaction/ETInteractionActor.h"
#include "ETFieldItem.generated.h"

class AETCharacter;
class UParticleSystemComponent;
struct FETFieldItemTableRowData;
struct FETGimmickTableRowData;

UCLASS()
class ET_API AETFieldItem : public AETInteractionActor
{
	GENERATED_BODY()

public:
	AETFieldItem();

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	virtual void PostInitializeComponents() override;

public:
	// IETInteractionInterface
	virtual void OnInteraction(AActor* InInteractor) override;
	// ~IETInteractionInterface

private:
	void ApplyFieldItemRowData(const FETFieldItemTableRowData* InFieldItemRowData);

	// 다시 상호작용하지 않도록 막고 유예 뒤 발동 (유예 동안 경고 파티클)
	void StartGimmick(AETCharacter* InInteractorCharacter, const FETGimmickTableRowData* InGimmickRowData);
	// 폭발, 스폰 볼륨 생성, 순간이동 후 제거
	void TriggerGimmick();

	// 경고 파티클, 폭발 기준 위치 (메시 바운드 중심, 메시가 없으면 액터 위치)
	FVector GetGimmickCenter() const;

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UParticleSystemComponent> ParticleSystemComponent;

	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetFieldItemRowNameOptions"))
	FName FieldItemRowName;

private:
	// 데이터 테이블 행 (테이블 에셋이 유지되는 동안 유효)
	const FETGimmickTableRowData* CachedGimmickRowData = nullptr;

	// 기믹을 발동시킨 캐릭터 (폭발 데미지 Instigator, 생성한 몬스터의 인식 대상)
	TWeakObjectPtr<AETCharacter> GimmickInstigatorCharacter;

	FTimerHandle GimmickTimerHandle;
};
