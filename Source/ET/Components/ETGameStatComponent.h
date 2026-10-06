#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "DataAsset/ETGameStatTableRowData.h"
#include "ETGameStatComponent.generated.h"

#define ET_GAME_STAT_VALUE_FUNCTIONS(Name) \
	FORCEINLINE void Add##Name(const float InValue) { Name += InValue; } \
	FORCEINLINE void Set##Name(const float InValue) { Name = InValue; } \
	FORCEINLINE void Reset##Name() { Name = 0.f; }

USTRUCT()
struct FETGameStat
{
	GENERATED_BODY()

public:	
	FETGameStat() = default;
	FETGameStat(const EETGameStatType InStatType) : StatType(InStatType) { }
	FETGameStat(const EETGameStatType InStatType, const float InBaseValue)
		: StatType(InStatType), BaseValue(InBaseValue)
	{
	}

	ET_GAME_STAT_VALUE_FUNCTIONS(PermanentValue);
	ET_GAME_STAT_VALUE_FUNCTIONS(ModifierValue);
	ET_GAME_STAT_VALUE_FUNCTIONS(DepletedValue);

	bool IsValid() const { return StatType != EETGameStatType::Max; }
	float GetMaxValue() const { return BaseValue + PermanentValue + ModifierValue; }
	float GetCurrentValue() const { return BaseValue + PermanentValue + ModifierValue + DepletedValue; }
	
private:
	EETGameStatType StatType = EETGameStatType::Max;
		
	float BaseValue = 0.f;       // 원본 기본값
	float PermanentValue = 0.f;  // 영구적인 증감값
	float ModifierValue = 0.f;   // 일시적인 능력치 보정값
	float DepletedValue = 0.f;   // 현재 상태에 의한 변동값
	
public:
	static const FETGameStat Invalid;
};

DECLARE_MULTICAST_DELEGATE_OneParam(FOnUpdateGameStat, const FETGameStat& InGameStat);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class ET_API UETGameStatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UETGameStatComponent();

protected:
	virtual void InitializeComponent() override;
	virtual void BeginPlay() override;

	void GenerateGameStat();

public:	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	const FETGameStat& GetGameStat(const EETGameStatType InGameStatType) const;
	void AddDepletedValue(const EETGameStatType InGameStatType, const float InValue);

protected:
	UPROPERTY(EditAnywhere)
	FName BaseGameStatRowName;

	UPROPERTY(VisibleAnywhere, Transient)
	TMap<EETGameStatType, FETGameStat> GameStatMap;

public:
	FOnUpdateGameStat OnUpdateGameStat;
};
