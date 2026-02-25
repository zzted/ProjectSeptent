// ZZ

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemGlobals.h"
#include "SeptentAbilitySystemGlobals.generated.h"

/**
 * 
 */
UCLASS()
class SEPTENT_API USeptentAbilitySystemGlobals : public UAbilitySystemGlobals
{
	GENERATED_BODY()

	virtual FGameplayEffectContext* AllocGameplayEffectContext() const override;
};
