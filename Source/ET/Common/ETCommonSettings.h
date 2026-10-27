#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UI/ETStatListWidget.h"
#include "ETCommonSettings.generated.h"

class UETPlayerHUDWidget;
class UETInteractionWidget;
class UETCheckpointWidget;
class UETMenuWidget;
class UETUserWidget;
class UETDamageTextWidget;
class UETThrobberWidget;
class UMaterialInterface;
class UWorld;

USTRUCT()
struct FETUISettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UETPlayerHUDWidget> PlayerHUDWidgetClass;

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UETInteractionWidget> InteractionWidgetClass;

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UETCheckpointWidget> CheckpointWidgetClass;

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UETMenuWidget> MenuWidgetClass;

	// 몬스터 머리 위 스탯 바
	UPROPERTY(EditAnywhere)
	FETStatListSettings MonsterStatList;

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UETUserWidget> LockOnWidgetClass;

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UETDamageTextWidget> DamageTextWidgetClass;

	UPROPERTY(EditAnywhere)
	TSoftClassPtr<UETThrobberWidget> ThrobberWidgetClass;
};

USTRUCT()
struct FETCharacterSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UMaterialInterface> AfterImageMaterial;
		
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UMaterialInterface> TimeFreezeMaterial;

	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UMaterialInterface> DissolveMaterial;
};

USTRUCT()
struct FETLevelSettings
{
	GENERATED_BODY()

public:
	// 새로하기, 진행 기록이 없을 때 시작할 레벨
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetLevelRowNameOptions"))
	FName StartLevelRowName;

	// 메인 타이틀 레벨 (플레이 레벨이 아니므로 레벨 테이블에 넣지 않음)
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> TitleLevel;

	// 마지막 레벨 클리어 후 포탈로 열 엔딩 레벨 (플레이 레벨이 아니므로 레벨 테이블에 넣지 않음)
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UWorld> EndingLevel;
};

// Project Settings > ET > Common > UI, Character...
UCLASS(Config = Game, DefaultConfig, meta = (DisplayName = "Common"))
class ET_API UETCommonSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UETCommonSettings();

public:
	UPROPERTY(Config, EditAnywhere, Category = "UI", meta = (ShowOnlyInnerProperties))
	FETUISettings UI;

	UPROPERTY(Config, EditAnywhere, Category = "Character", meta = (ShowOnlyInnerProperties))
	FETCharacterSettings Character;

	UPROPERTY(Config, EditAnywhere, Category = "Level", meta = (ShowOnlyInnerProperties))
	FETLevelSettings Level;
};
