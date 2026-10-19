#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UI/ETStatListWidget.h"
#include "ETCommonSettings.generated.h"

class UETPlayerHUDWidget;
class UETInteractionWidget;
class UETCheckpointWidget;
class UETUserWidget;
class UETDamageTextWidget;
class UETThrobberWidget;
class UMaterialInterface;

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
};
