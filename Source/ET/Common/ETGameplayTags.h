#pragma once

#include "NativeGameplayTags.h"

namespace ETGameplayTags
{
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Idle);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Attacking);		
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_HeavyAttacking);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_State_Death);

	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Action_ComboAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Action_JumpAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Action_HeavyAttack);
	UE_DECLARE_GAMEPLAY_TAG_EXTERN(Character_Action_Hit);
}
