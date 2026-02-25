// ZZ

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "LevelUpInfo.generated.h"

USTRUCT(BlueprintType)
struct FSeptentLevelUpInfo
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	int32 LevelUpRequirement = 0;

	UPROPERTY(EditDefaultsOnly)
	int32 AttributePointReward = 1;

	UPROPERTY(EditDefaultsOnly)
	int32 SpellPointReward = 1;
};

/**
 * 
 */
UCLASS()
class SEPTENT_API ULevelUpInfo : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Level")
	TArray<FSeptentLevelUpInfo> LevelUpInformation;

	int32 FindLevelForXP(const int32 XP) const;
};
