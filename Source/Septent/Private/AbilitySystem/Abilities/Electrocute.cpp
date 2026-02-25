// ZZ


#include "AbilitySystem/Abilities/Electrocute.h"

FString UElectrocute::GetDescription(int32 Level)
{
	const int32 Damage = DamageAmount.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float CoolDown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(""
							  "<Title>ELECTROCUTE</>\n\n"
							  
							  "<Small>Level: </><Level>%d</>\n"
							  "<Small>Mana Cost: </><ManaCost>%.1f</>\n" // %.1f will have only one decimal
							  "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
							  
							  "<Default>Emits a beam of lightning, causing: </> <Damage>%d</> <Default> lightning damage with a chance to stun</>\n\n"
							  ),  Level, ManaCost, CoolDown, Damage);
	}
	return FString::Printf(TEXT(""
		"<Title>ELECTROCUTE</>\n\n"
		
		"<Small>Level: </><Level>%d</>\n"
		"<Small>Mana Cost: </><ManaCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		
		"<Default>Emits a beam of lightning, propagating to %d additional targets nearby, causing: </> <Damage>%d</> <Default> lightning damage with a chance to stun</>\n\n"
		), Level, ManaCost, CoolDown, FMath::Min(Level, MaxNumShockTargets) - 1, Damage);
}

FString UElectrocute::GetNextLevelDescription(int32 Level)
{
	const int32 Damage = DamageAmount.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float CoolDown = GetCooldown(Level);

	return FString::Printf(TEXT(""
		"<Title>NEXT LEVEL:</>\n\n"
		
		"<Small>Level: </><Level>%d</>\n"
		"<Small>Mana Cost: </><ManaCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		
		"<Default>Emits a beam of lightning, propagating to %d additional targets nearby, causing: </> <Damage>%d</> <Default> lightning damage with a chance to stun</>\n\n"
		), Level, ManaCost, CoolDown, FMath::Min(Level, MaxNumShockTargets) - 1, Damage);
}
