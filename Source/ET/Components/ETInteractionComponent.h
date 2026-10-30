#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "ETInteractionComponent.generated.h"

class AETCharacter;
class UAnimInstance;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETInteractionComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UETInteractionComponent();

protected:	
	virtual void BeginPlay() override;

public:		
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	
public:
	void SetInteractionTarget(AActor* InTarget);
	void ClearInteractionTarget();
	AActor* GetInteractionTarget() const { return InteractionTarget.Get(); }

	// 대상이 있고 행동 가능하면 상호작용 몽타주 재생 (실제 상호작용은 몽타주의 ETAnimNotify_Interaction 시점)
	bool StartInteraction();
	// 대상의 상호작용 실행, 대기하는 대상이면 몽타주 정지
	void DoInteraction();
	void DoInteraction(AActor* InTarget);
	// 대기 중이던 상호작용 몽타주 재개 (체크포인트 창 닫기 등)
	void EndInteraction();
	// 상호작용 몽타주를 바로 블렌드아웃 (기믹 발동 등 위협이 생겨 남은 동작을 기다리지 않고 조작을 돌려줄 때)
	// bInImmediately 면 블렌드 없이 바로 정지 (순간이동 등 다른 위치에서 기본 자세로 시작할 때)
	void CancelInteraction(const bool bInImmediately = false);

private:
	void ReleaseInteractionTarget();
	AETCharacter* GetOwnerCharacter() const;
	UAnimInstance* GetInteractingAnimInstance() const;

private:
	TWeakObjectPtr<AActor> InteractionTarget;
};
