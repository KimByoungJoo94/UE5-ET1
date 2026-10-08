#include "DataAsset/ETCollisionTableRowData.h"

FETWeaponCollisionTableRowData::FETWeaponCollisionTableRowData()
{
	ObjectTypeArray.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
}

FETRadialAttackCollisionTableRowData::FETRadialAttackCollisionTableRowData()
{
	ObjectTypeArray.Add(UEngineTypes::ConvertToObjectType(ECC_Pawn));
}
