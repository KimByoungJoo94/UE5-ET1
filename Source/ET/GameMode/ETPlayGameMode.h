// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "ETPlayGameMode.generated.h"

class AETCheckpoint;

UCLASS()
class ET_API AETPlayGameMode : public AGameModeBase
{
	GENERATED_BODY()

protected:
	virtual void BeginPlay() override;

public:
	// 마지막 체크포인트가 레벨에 있으면 그 부활 위치에서, 없으면 기본 PlayerStart 에서 시작
	virtual void RestartPlayer(AController* NewPlayer) override;

protected:
	AETCheckpoint* FindLastCheckpoint() const;
};
