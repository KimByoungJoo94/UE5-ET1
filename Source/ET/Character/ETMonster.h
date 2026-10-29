#pragma once

#include "CoreMinimal.h"
#include "Character/ETCharacter.h"
#include "Interface/ETLockOnInterface.h"
#include "ETMonster.generated.h"

class UWidgetComponent;
class UETDissolveComponent;
class AETSpawnVolume;
class UBlackboardComponent;

UCLASS()
class ET_API AETMonster : public AETCharacter, public IETLockOnInterface
{
	GENERATED_BODY()

public:
	AETMonster();

protected:
	virtual void PostInitializeComponents() override;
	virtual void BeginPlay() override;
	virtual void Die() override;
	virtual void Stun() override;
	virtual void OnStunEnded(bool bInterrupted) override;
	virtual void OnIntroEnded(bool bInterrupted) override;

public:
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser) override;
	virtual AActor* GetAttackTarget() const override;

	// 대상을 처음 인식하면 전투 전 등장 연출 (스폰당 한 번, Intro 액션 데이터가 있는 몬스터만)
	virtual bool PlayIntro() override;

	// 대상 인식 (이미 같은 대상이면 무시, 같은 스폰 볼륨 몬스터도 연쇄 인식)
	// bInPlayIntro 면 행동 중이 아닐 때 등장 연출 (피격으로 인식하면 연출 없이 바로 전투)
	void DetectTarget(AActor* InTarget, const bool bInPlayIntro);
	void LoseTarget();

	AETSpawnVolume* GetHomeSpawnVolume() const;
	// 생성한 스폰 볼륨이 FinishSpawning 전에 지정
	void SetHomeSpawnVolume(AETSpawnVolume* InHomeSpawnVolume);

	// IETLockOnInterface
	virtual void OnLockOnTargeted() override;
	virtual void OnLockOnTargetReleased() override;
	// ~IETLockOnInterface

protected:
	// 디졸브 완료 후 사라짐 처리 (풀링 / 리스폰 도입 시 여기만 교체)
	void Disappear();

	void OnDissolveFinished();

	// 대상을 인식 중이거나 플레이어가 락온 중일 때만 표시 (사망 후에는 항상 숨김)
	void UpdateStatBarVisible();

	// BT 액션 태스크가 몽타주를 끊지 않도록 사망, 스턴, 등장 연출 동안 정지 (블랙보드는 유지)
	void StopBrainLogic(const FString& InReason);
	void RestartBrainLogic();

	UBlackboardComponent* GetBlackboardComponent() const;

protected:
	// 머리 위 스탯 바 (보스는 사용하지 않으므로 BeginPlay 에서 제거)
	UPROPERTY(VisibleAnywhere, Category = "ET")
	TObjectPtr<UWidgetComponent> StatBarWidgetComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET")
	TObjectPtr<UWidgetComponent> LockOnWidgetComponent;

	UPROPERTY(VisibleAnywhere, Category = "ET|Component")
	TObjectPtr<UETDissolveComponent> DissolveComponent;

private:
	TWeakObjectPtr<AETSpawnVolume> HomeSpawnVolume;

	bool bIntroPlayed = false;

	bool bLockOnTargeted = false;
};
