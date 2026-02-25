// ZZ


#include "AbilitySystem/Abilities/PassiveAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystem/SeptentAbilitySystemComponent.h"

void UPassiveAbility::ActivateAbility(const FGameplayAbilitySpecHandle Handle,
                                          const FGameplayAbilityActorInfo* ActorInfo, const FGameplayAbilityActivationInfo ActivationInfo,
                                          const FGameplayEventData* TriggerEventData)
{
	Super::ActivateAbility(Handle, ActorInfo, ActivationInfo, TriggerEventData);

	if (USeptentAbilitySystemComponent* SeptentASC = Cast<USeptentAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetAvatarActorFromActorInfo())))
	{
		SeptentASC->DeactivatePassiveAbilityDelegate.AddUObject(this, &UPassiveAbility::ReceiveDeactivate);
	}
}

void UPassiveAbility::ReceiveDeactivate(const FGameplayTag& AbilityTag)
{
	if (AbilityTags.HasTagExact(AbilityTag))
	{
		EndAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true, true);
	}
}
