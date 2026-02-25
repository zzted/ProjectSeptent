// ZZ

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Abilities/SeptentGameplayAbility.h"
#include "PassiveAbility.generated.h"

/**
 * 
 */
UCLASS()
class SEPTENT_API UPassiveAbility : public USeptentGameplayAbility
{
	GENERATED_BODY()

public:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	void ReceiveDeactivate(const FGameplayTag& AbilityTag);
};
