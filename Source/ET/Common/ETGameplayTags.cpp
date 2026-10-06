#include "Common/ETGameplayTags.h"


namespace ETGameplayTags
{
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Idle, "Character.State.Idle");
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Attacking, "Character.State.Attacking");	
	UE_DEFINE_GAMEPLAY_TAG(Character_State_HeavyAttacking, "Character.State.HeavyAttacking");
	UE_DEFINE_GAMEPLAY_TAG(Character_State_Death, "Character.State.Death");

	UE_DEFINE_GAMEPLAY_TAG(Character_Action_ComboAttack, "Character.Action.ComboAttack");
	UE_DEFINE_GAMEPLAY_TAG(Character_Action_JumpAttack, "Character.Action.JumpAttack");	
	UE_DEFINE_GAMEPLAY_TAG(Character_Action_HeavyAttack, "Character.Action.HeavyAttack");
	UE_DEFINE_GAMEPLAY_TAG(Character_Action_Hit, "Character.Action.Hit");
}
