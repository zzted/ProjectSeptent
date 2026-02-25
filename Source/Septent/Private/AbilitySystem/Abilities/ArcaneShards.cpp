// ZZ


#include "AbilitySystem/Abilities/ArcaneShards.h"

FString UArcaneShards::GetDescription(int32 Level)
{
	const int32 Damage = DamageAmount.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float CoolDown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(""
							  "<Title>ARCANE SHARD</>\n\n"
							  
							  "<Small>Level: </><Level>%d</>\n"
							  "<Small>Mana Cost: </><ManaCost>%.1f</>\n" // %.1f will have only one decimal
							  "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
							  
							  "<Default>Summon a shard of arcane energy, causing: </> <Damage>%d</> <Default> radial arcane damage with a chance to stun</>\n\n"
							  ),  Level, ManaCost, CoolDown, Damage);
	}
	return FString::Printf(TEXT(""
		"<Title>ARCANE SHARDS</>\n\n"
		
		"<Small>Level: </><Level>%d</>\n"
		"<Small>Mana Cost: </><ManaCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		
		"<Default>Summon %d shards of arcane enegy, causing: </> <Damage>%d</> <Default> radial arcane damage with a chance to stun</>\n\n"
		), Level, ManaCost, CoolDown, FMath::Min(Level, MaxNumShards), Damage);
}

FString UArcaneShards::GetNextLevelDescription(int32 Level)
{
	const int32 Damage = DamageAmount.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float CoolDown = GetCooldown(Level);
	
	return FString::Printf(TEXT(""
		"<Title>ARCANE SHARDS</>\n\n"
		
		"<Small>Level: </><Level>%d</>\n"
		"<Small>Mana Cost: </><ManaCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		
		"<Default>Summon %d shards of arcane enegy, causing: </> <Damage>%d</> <Default> radial arcane damage with a chance to stun</>\n\n"
		), Level, ManaCost, CoolDown, FMath::Min(Level, MaxNumShards), Damage);
}
