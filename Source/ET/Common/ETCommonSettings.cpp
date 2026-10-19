#include "Common/ETCommonSettings.h"
#include "UI/ETPlayerHUDWidget.h"
#include "UI/ETInteractionWidget.h"
#include "UI/ETCheckpointWidget.h"
#include "Materials/MaterialInterface.h"

UETCommonSettings::UETCommonSettings()
{
	CategoryName = TEXT("ET");

	UI.MonsterStatList.StatBarDataArray =
	{
		{ EETGameStatType::Health, TEXT("red") },
		{ EETGameStatType::StunPoint, TEXT("yellow") },
	};
}
