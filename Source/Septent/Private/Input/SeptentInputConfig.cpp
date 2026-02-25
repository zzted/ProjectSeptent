// ZZ


#include "Input/SeptentInputConfig.h"

const UInputAction* USeptentInputConfig::FindAbilityInputActionForTag(const FGameplayTag& InputTag,
                                                                      bool bLogNotFound) const
{
	for (FSeptentInputAction SeptentInputAction : AbilityInputActions)
	{
		if (SeptentInputAction.InputAction && SeptentInputAction.InputTag.MatchesTagExact(InputTag))
		{
			return SeptentInputAction.InputAction;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogTemp, Error, TEXT("Could not find AbilityInputAction action for InputTag [%s], on SeptentInputConfig %s"), *InputTag.ToString(), *GetNameSafe(this));
	}

	return nullptr;
}
