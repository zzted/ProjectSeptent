// ZZ


#include "AbilitySystem/SeptentAbilitySystemGlobals.h"

#include "SeptentAbilitySystemTypes.h"

FGameplayEffectContext* USeptentAbilitySystemGlobals::AllocGameplayEffectContext() const
{
	return new FSeptentGameplayEffectContext();
}
