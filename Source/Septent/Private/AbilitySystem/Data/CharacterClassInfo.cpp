// ZZ


#include "AbilitySystem/Data/CharacterClassInfo.h"

FCharacterClassDefaultInfo UCharacterClassInfo::GetDefaultInfo(ECharacterClass CharacterClass) const
{
	return CharacterClassInfo.FindChecked(CharacterClass);
}
