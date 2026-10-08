#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "ETCommonSettings.generated.h"

class UETPlayerHUDWidget;
class UETInteractionWidget;
class UETCheckpointWidget;
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
};

USTRUCT()
struct FETCharacterSettings
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UMaterialInterface> AfterImageMaterial;

	// TODO : 시간 정지 대상에 덮어씌울 머터리얼 (ETTimeFreezeComponent)
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UMaterialInterface> TimeFreezeMaterial;
};

// Project Settings > ET > Common (섹션별 구조체로 구분)
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
