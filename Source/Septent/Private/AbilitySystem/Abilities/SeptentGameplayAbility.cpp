// ZZ


#include "AbilitySystem/Abilities/SeptentGameplayAbility.h"

#include "AbilitySystem/SeptentAttributeSet.h"

FString USeptentGameplayAbility::GetDescription(const int32 Level)
{
	return FString::Printf(TEXT("<Default> %s, </><Level>%d</>"), L"Default Ability Name - LoremIpsumLoremIpsumLoremIpsumLoremIpsumLoremIpsum", Level);
}

FString USeptentGameplayAbility::GetNextLevelDescription(const int32 Level)
{
	return FString::Printf(TEXT("<Default> %s, </><Level>%d</> \n <Default> teng </>"), L"Default Ability Name - LoremIpsumLoremIpsumLoremIpsumLoremIpsumLoremIpsum", Level);
}

FString USeptentGameplayAbility::GetLockedDescription(const int32 Level)
{
	return FString::Printf(TEXT("<Default> Spell Locked until </><Level>%d</>"), Level);
}

float USeptentGameplayAbility::GetManaCost(float InLevel) const
{
	float ManaCost = 0.f;
	if (const UGameplayEffect* CostEffect = GetCostGameplayEffect())
	{
		for (FGameplayModifierInfo Mod : CostEffect->Modifiers)
		{
			if (Mod.Attribute == USeptentAttributeSet::GetManaAttribute())
			{
				Mod.ModifierMagnitude.GetStaticMagnitudeIfPossible(InLevel, ManaCost);
				break;
			}
		}
	}
	return ManaCost;
}

float USeptentGameplayAbility::GetCooldown(float InLevel) const
{
	float Cooldown = 0.f;
	if (const UGameplayEffect* CooldownEffect = GetCooldownGameplayEffect())
	{
		CooldownEffect->DurationMagnitude.GetStaticMagnitudeIfPossible(InLevel, Cooldown);
	}
	return Cooldown;
}
