// ZZ


#include "AbilitySystem/Data/AttributeInfo.h"

#include "Septent/SeptentLogChannels.h"

FSeptentAttributeInfo UAttributeInfo::FindAttributeInfoForTag(const FGameplayTag& AttributeTag, bool bLogNotFound) const
{
	for (const FSeptentAttributeInfo& AttributeInfo : AttributeInfos)
	{
		if (AttributeInfo.AttributeTag.MatchesTagExact(AttributeTag))
		{
			return AttributeInfo;
		}
	}

	if (bLogNotFound)
	{
		UE_LOG(LogSeptent, Error, TEXT("Could not find attribute info for tag %s, on AttributeInfo %s"), *AttributeTag.ToString(), *GetNameSafe(this));
	}

	return FSeptentAttributeInfo();
}
