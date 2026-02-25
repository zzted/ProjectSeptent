// ZZ

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "Engine/DataAsset.h"
#include "SeptentInputConfig.generated.h"

USTRUCT(BlueprintType)
struct FSeptentInputAction
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly)
	const class UInputAction* InputAction = nullptr;

	UPROPERTY(EditDefaultsOnly)
	FGameplayTag InputTag = FGameplayTag();
};

/**
 * @class USeptentInputConfig
 * @brief Represents a configuration asset for input mappings in the Septent game.
 *
 * This class derives from UDataAsset and is intended for use in defining configurable input-related setups.
 * It acts as a container asset for input mapping information that can be utilized across different parts of the game.
 * The configuration defined in this asset can affect how input is handled or customized in the gameplay experience.
 */
UCLASS()
class SEPTENT_API USeptentInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:

	const UInputAction* FindAbilityInputActionForTag(const FGameplayTag& InputTag, bool bLogNotFound = false) const;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TArray<FSeptentInputAction> AbilityInputActions;
};
