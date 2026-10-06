#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "ETAnimInstance.generated.h"

class AETCharacter;

UCLASS()
class ET_API UETAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
	
public:
	UETAnimInstance();

	virtual void NativeInitializeAnimation() override;
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;

protected:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ET")
	FRotator RotationLastTick;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ET")
	float YawDelta;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ET")
	float Speed;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ET")
	bool bInAir;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ET")
	bool bAccelerating;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "ET")
	TObjectPtr<AETCharacter> Character;


	
	/*
	float Yaw;
	float Pitch;
	float Roll;
	
	

	
	

	

	bool bAttacking;
	int32 CurrentAttack;
	bool bFullBody;
	*/

};
