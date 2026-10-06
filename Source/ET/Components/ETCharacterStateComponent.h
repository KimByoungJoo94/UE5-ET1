#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Common/ETGameplayTags.h"
#include "ETCharacterStateComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETCharacterStateComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UETCharacterStateComponent();

protected:
	virtual void BeginPlay() override;

public:		
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FORCEINLINE const FGameplayTag GetCurrentState() const { return CurrentState; };
	FORCEINLINE bool IsCurrentState(const FGameplayTag InState) const { return CurrentState == InState; }
	FORCEINLINE bool HasCurrentState(const FGameplayTagContainer& InStateContainer) const { return InStateContainer.HasTagExact(CurrentState); }
	FORCEINLINE bool ConatinsCurrentState(const TSet<FGameplayTag>& InStateSet) const { return InStateSet.Contains(CurrentState); }

	void ChangeState(const FGameplayTag InNewState);
	void ClearState();
	
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "State")
	FGameplayTag CurrentState;
};
