#pragma once

#include "CoreMinimal.h"
#include "Engine/DataTable.h"
#include "ETFieldItemTableRowData.generated.h"

class UParticleSystem;

// 레벨에 배치하는 아이템 종류 (보상 + 외형, 위치 조정이 필요한 외형은 BP 에서)
USTRUCT()
struct ET_API FETFieldItemTableRowData : public FTableRowBase
{
	GENERATED_BODY()

public:
	// 비어 있으면 보상 없음
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetRewardRowNameOptions"))
	FName RewardRowName;

	// 비어 있으면 기믹 없음 (보상과 독립, 보상은 상호작용 즉시 지급)
	UPROPERTY(EditAnywhere, meta = (GetOptions = "ET.ETGameDataSubsystem.GetGimmickRowNameOptions"))
	FName GimmickRowName;

	// 비어 있으면 BP 설정 유지
	UPROPERTY(EditAnywhere)
	TSoftObjectPtr<UParticleSystem> ParticleSystem;
};
