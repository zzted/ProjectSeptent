// ZZ


#include "AbilitySystem/Data/AbilityInfo.h"

#include "Septent/SeptentLogChannels.h"

FSeptentAbilityInfo UAbilityInfo::FindAbilityInfoForTag(const FGameplayTag& AbilityTag, bool bLogNotFound) const
{
	for (const FSeptentAbilityInfo& Info : AbilityInformation)
	{
		if (Info.AbilityTag.MatchesTagExact(AbilityTag))
		{
			return Info;
		}
	}

	if (bLogNotFound)
	{
		//UE_LOG(LogTemp, Error, TEXT("Could not find ability info for tag %s, on AbilityInfo %s"), *AbilityTag.ToString(), *GetNameSafe(this));
		UE_LOG(LogSeptent, Error, TEXT("Could not find ability info for tag [%s], on AbilityInfo [%s]"), *AbilityTag.ToString(), *GetNameSafe(this));
	}

	return FSeptentAbilityInfo();
}
