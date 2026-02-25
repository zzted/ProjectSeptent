// ZZ


#include "AbilitySystem/SeptentAbilitySystemComponent.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "SeptentGameplayTags.h"
#include "AbilitySystem/SeptentAbilitySystemLibrary.h"
#include "AbilitySystem/Abilities/SeptentGameplayAbility.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Septent/SeptentLogChannels.h"
#include "Game/LoadScreenSaveGame.h"
#include "Interaction/PlayerInterface.h"

void USeptentAbilitySystemComponent::AbilityActorInfoSet()
{
	OnGameplayEffectAppliedDelegateToSelf.AddUObject(this, &USeptentAbilitySystemComponent::ClientEffectApplied);
}

void USeptentAbilitySystemComponent::AddCharacterAbilities(const TArray<TSubclassOf<UGameplayAbility>>& StartupAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		if (const USeptentGameplayAbility* Ability = Cast<USeptentGameplayAbility>(AbilitySpec.Ability.Get()))
		{
			FGameplayTagContainer& DynamicTags = AbilitySpec.GetDynamicSpecSourceTags();
			DynamicTags.AddTag(Ability->StartupInputTag);
			DynamicTags.AddTag(FSeptentGameplayTags::Get().Abilities_Status_Equipped);
			GiveAbility(AbilitySpec);
		}
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast(); // only broadcast on server, overriding OnRep_ActivateAbilities to broadcast on client as well 
}

void USeptentAbilitySystemComponent::AddCharacterPassiveAbilities(
	const TArray<TSubclassOf<UGameplayAbility>>& StartupPassiveAbilities)
{
	for (const TSubclassOf<UGameplayAbility> AbilityClass : StartupPassiveAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		FGameplayTagContainer& DynamicTags = AbilitySpec.GetDynamicSpecSourceTags();
		DynamicTags.AddTag(FSeptentGameplayTags::Get().Abilities_Status_Equipped);
		GiveAbilityAndActivateOnce(AbilitySpec);
	}
}

void USeptentAbilitySystemComponent::AddCharacterAbilitiesFromSaveData(ULoadScreenSaveGame* SaveData)
{
	FSeptentGameplayTags GameplayTags = FSeptentGameplayTags::Get();
	for (const FSavedAbility& Data : SaveData->SavedAbilities)
	{
		const TSubclassOf<UGameplayAbility> LoadedAbilityClass = Data.GameplayAbility;

		FGameplayAbilitySpec LoadedAbilitySpec = FGameplayAbilitySpec(LoadedAbilityClass, Data.AbilityLevel);
		FGameplayTagContainer& DynamicTags = LoadedAbilitySpec.GetDynamicSpecSourceTags();
		DynamicTags.AddTag(Data.AbilitySlot);
		DynamicTags.AddTag(Data.AbilityStatus);
		if (Data.AbilityType == GameplayTags.Abilities_Type_Offensive)
		{
			GiveAbility(LoadedAbilitySpec);
		}
		else if (Data.AbilityType == GameplayTags.Abilities_Type_Passive)
		{
			if (Data.AbilityStatus.MatchesTagExact(GameplayTags.Abilities_Status_Equipped))
			{
				GiveAbilityAndActivateOnce(LoadedAbilitySpec);
			}
			else
			{
				GiveAbility(LoadedAbilitySpec);
			}
		}
	}
	bStartupAbilitiesGiven = true;
	AbilitiesGivenDelegate.Broadcast(); // only broadcast on server, overriding OnRep_ActivateAbilities to broadcast on client as well 
}

void USeptentAbilitySystemComponent::AbilityInputTagHeld(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLock(*this); 
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		FGameplayTagContainer& DynamicTags = AbilitySpec.GetDynamicSpecSourceTags();
		if (DynamicTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (!AbilitySpec.IsActive())
			{
				TryActivateAbility(AbilitySpec.Handle);
			}
		}
	}
	
}

void USeptentAbilitySystemComponent::AbilityInputTagPressed(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLock(*this); 
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		FGameplayTagContainer& DynamicTags = AbilitySpec.GetDynamicSpecSourceTags();
		if (DynamicTags.HasTagExact(InputTag))
		{
			AbilitySpecInputPressed(AbilitySpec);
			if (AbilitySpec.IsActive())
			{
// 				if (const UGameplayAbility* PrimaryInstance = AbilitySpec.GetPrimaryInstance())
// 				{
// 					// Sending events across the net work for the Ability Tasks to work.
// 					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, 
// PrimaryInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
// 				}
				TArray<UGameplayAbility*> AbilityInstances = AbilitySpec.GetAbilityInstances();
				for (UGameplayAbility* AbilityInstance : AbilityInstances)
				{
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputPressed, AbilitySpec.Handle, AbilityInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
				}
			}
		}
	}
}

void USeptentAbilitySystemComponent::AbilityInputTagReleased(const FGameplayTag& InputTag)
{
	if (!InputTag.IsValid()) return;

	FScopedAbilityListLock ActiveScopeLock(*this); 
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(InputTag) && AbilitySpec.IsActive())
		{
			AbilitySpecInputReleased(AbilitySpec);
			if (AbilitySpec.IsActive())
			{
				if (const UGameplayAbility* PrimaryInstance = AbilitySpec.GetPrimaryInstance())
				{
					// Added to make wait Input Release task in BP work
					InvokeReplicatedEvent(EAbilityGenericReplicatedEvent::InputReleased, AbilitySpec.Handle, 
						PrimaryInstance->GetCurrentActivationInfo().GetActivationPredictionKey());
				}
			}
		}
	}
}

void USeptentAbilitySystemComponent::ForEachAbility(const FForEachAbility& Delegate)
{
	FScopedAbilityListLock ActiveScopeLock(*this); // lock the list of abilities before this loop has finished
	for (const FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (!Delegate.ExecuteIfBound(AbilitySpec))
		{
			UE_LOG(LogSeptent, Error, TEXT("Failed to execute delegate in %hs"), __FUNCTION__);
		}
	}
}

FGameplayTag USeptentAbilitySystemComponent::GetAbilityTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	if (AbilitySpec.Ability)
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities")))) // check if it has a tag that belongs to ability, like Abilities.Fire.FireBolt
			{
				return Tag;
			}
		}
	}
	return FGameplayTag();
}

FGameplayTag USeptentAbilitySystemComponent::GetInputTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	const FGameplayTagContainer& DynamicTags = AbilitySpec.GetDynamicSpecSourceTags();
	for (FGameplayTag Tag : DynamicTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("InputTag"))))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

FGameplayTag USeptentAbilitySystemComponent::GetStatusTagFromSpec(const FGameplayAbilitySpec& AbilitySpec)
{
	const FGameplayTagContainer& DynamicTags = AbilitySpec.GetDynamicSpecSourceTags();
	for (FGameplayTag Tag : DynamicTags)
	{
		if (Tag.MatchesTag(FGameplayTag::RequestGameplayTag(FName("Abilities.Status"))))
		{
			return Tag;
		}
	}
	return FGameplayTag();
}

FGameplayTag USeptentAbilitySystemComponent::GetStatusTagFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetStatusTagFromSpec(*AbilitySpec);
	}
	return FGameplayTag();
}

FGameplayTag USeptentAbilitySystemComponent::GetInputTagFromAbilityTag(const FGameplayTag& AbilityTag)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		return GetInputTagFromSpec(*AbilitySpec);
	}
	return FGameplayTag();
}

FGameplayAbilitySpec* USeptentAbilitySystemComponent::GetSpecFromAbilityTag(const FGameplayTag& AbilityTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this); // lock the list of abilities before this loop has finished
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		for (FGameplayTag Tag : AbilitySpec.Ability.Get()->AbilityTags)
		{
			if (Tag.MatchesTag(AbilityTag))
			{
				return &AbilitySpec;
			}
		}
	}
	return nullptr;
}

bool USeptentAbilitySystemComponent::SlotIsEmpty(const FGameplayTag& SlotTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this); 
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(AbilitySpec, SlotTag))
		{
			return false;
		}
	}
	return true;
}

bool USeptentAbilitySystemComponent::AbilityHasSlot(const FGameplayAbilitySpec& AbilitySpec, const FGameplayTag& SlotTag)
{
	return AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(SlotTag);
}

bool USeptentAbilitySystemComponent::AbilityHasAnySlot(const FGameplayAbilitySpec& AbilitySpec)
{
	return AbilitySpec.GetDynamicSpecSourceTags().HasTag(FGameplayTag::RequestGameplayTag(FName("InputTag")));
}

FGameplayAbilitySpec* USeptentAbilitySystemComponent::GetAbilitySpecFromSlot(const FGameplayTag& SlotTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this); 
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilityHasSlot(AbilitySpec, SlotTag))
		{
			return &AbilitySpec;
		}
	}
	return nullptr;
}

bool USeptentAbilitySystemComponent::IsPassiveAbility(const FGameplayAbilitySpec& AbilitySpec) const
{
	const UAbilityInfo* AbilityInfo = USeptentAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	const FGameplayTag& AbilityTag = GetAbilityTagFromSpec(AbilitySpec);
	const FSeptentAbilityInfo& Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	const FGameplayTag AbilityType = Info.AbilityType;
	return AbilityType.MatchesTagExact(FSeptentGameplayTags::Get().Abilities_Type_Passive);
}

void USeptentAbilitySystemComponent::AssignSlotToAbility(FGameplayAbilitySpec* AbilitySpec, const FGameplayTag& SlotTag)
{
	ClearSlot(AbilitySpec);
	AbilitySpec->GetDynamicSpecSourceTags().AddTag(SlotTag);
}

void USeptentAbilitySystemComponent::MulticastActivatePassiveEffect_Implementation(const FGameplayTag& AbilityTag, bool bActivate)
{
	// UE_LOG(LogTemp, Warning, TEXT("MulticastActivatePassiveEffect received on %s Role=%d NetMode=%d, AbilityTag=%s, Activate=%d"),
	// 	*GetNameSafe(GetOwner()), (int)GetOwnerRole(), (int)GetNetMode(), *AbilityTag.ToString(), bActivate);
	ActivatePassiveEffect.Broadcast(AbilityTag, bActivate);
}

void USeptentAbilitySystemComponent::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	if (GetAvatarActor() && GetAvatarActor()->Implements<UPlayerInterface>())
	{
		if (IPlayerInterface::Execute_GetAttributePoints(GetAvatarActor()) > 0)
		{
			ServerUpgradeAttribute(AttributeTag);
		}
	}
}

void USeptentAbilitySystemComponent::ServerUpgradeAttribute_Implementation(const FGameplayTag& AttributeTag)
{
	FGameplayEventData Payload;
	Payload.EventTag = AttributeTag;
	Payload.EventMagnitude = 1.f;
	UAbilitySystemBlueprintLibrary::SendGameplayEventToActor(GetAvatarActor(), AttributeTag, Payload);

	if (GetAvatarActor() && GetAvatarActor()->Implements<UPlayerInterface>())
	{
		IPlayerInterface::Execute_AddToAttributePoints(GetAvatarActor(), -1);
	}
}

void USeptentAbilitySystemComponent::UpdateAbilityStatuses(int32 Level)
{
	UAbilityInfo* AbilityInfo = USeptentAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	for (FSeptentAbilityInfo& Info : AbilityInfo->AbilityInformation)
	{
		if (!Info.AbilityTag.IsValid()) continue;
		if (Level < Info.LevelRequirement) continue;
		if (!GetSpecFromAbilityTag(Info.AbilityTag))
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(Info.Ability, 1);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(Info.AbilityTag);
			AbilitySpec.GetDynamicSpecSourceTags().AddTag(FSeptentGameplayTags::Get().Abilities_Status_Eligible);
			GiveAbility(AbilitySpec);
			MarkAbilitySpecDirty(AbilitySpec); // Force Replicate
			ClientUpdateAbilityStatus(Info.AbilityTag, FSeptentGameplayTags::Get().Abilities_Status_Eligible, 1);
		}
	}
}

void USeptentAbilitySystemComponent::ServerSpendSpellPoint_Implementation(const FGameplayTag& AbilityTag)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag)) // eligible and unlocked abilities are activable  
	{
		if (GetAvatarActor() && GetAvatarActor()->Implements<UPlayerInterface>())
		{
			IPlayerInterface::Execute_AddToSpellPoints(GetAvatarActor(), -1);
		}
		
		const FSeptentGameplayTags GameplayTags = FSeptentGameplayTags::Get();
		FGameplayTag Status = GetStatusTagFromSpec(*AbilitySpec);
		if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Eligible))
		{
			AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GameplayTags.Abilities_Status_Eligible);
			AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Unlocked);
			Status = GameplayTags.Abilities_Status_Unlocked;
		}
		else if (Status.MatchesTagExact(GameplayTags.Abilities_Status_Equipped) || Status.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
		{
			AbilitySpec->Level++;
		}

		ClientUpdateAbilityStatus(AbilityTag, Status, AbilitySpec->Level);
		MarkAbilitySpecDirty(*AbilitySpec); // Force Replicate
	}
}

void USeptentAbilitySystemComponent::ServerEquipAbility_Implementation(const FGameplayTag& AbilityTag,
	const FGameplayTag& NewSlotTag /*SlotTag is the InputTag*/)
{
	if (FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		const FSeptentGameplayTags GameplayTags = FSeptentGameplayTags::Get();
		const FGameplayTag& PrevSlotTag = GetInputTagFromSpec(*AbilitySpec);
		FGameplayTag StatusTag = GetStatusTagFromSpec(*AbilitySpec);

		const bool bStatusValid = StatusTag == GameplayTags.Abilities_Status_Equipped || StatusTag == GameplayTags.Abilities_Status_Unlocked;
		if (bStatusValid)
		{
			// Handle activation/deactivation for passive abilities

			if (!SlotIsEmpty(NewSlotTag))
			{
				FGameplayAbilitySpec* SpecWithSlot = GetAbilitySpecFromSlot(NewSlotTag);
				if (SpecWithSlot)
				{
					// Return early if replacing with the same ability
					if (AbilityTag.MatchesTagExact(GetAbilityTagFromSpec(*SpecWithSlot)))
					{
						ClientEquipAbility(AbilityTag, StatusTag, NewSlotTag, PrevSlotTag);
						return;
					}

					if (IsPassiveAbility(*SpecWithSlot))
					{
						MulticastActivatePassiveEffect(GetAbilityTagFromSpec(*SpecWithSlot), false); // Deactivate the ability effect that's currently active in the slot
						DeactivatePassiveAbilityDelegate.Broadcast(GetAbilityTagFromSpec(*SpecWithSlot));
					}

					ClearSlot(SpecWithSlot);
				}
			}

			if (!AbilityHasAnySlot(*AbilitySpec)) // Ability doesn't yet have a slot (not active)
			{
				if (IsPassiveAbility(*AbilitySpec))
				{
					TryActivateAbility(AbilitySpec->Handle);
					// UE_LOG(LogTemp, Warning, TEXT("[Server] ServerEquipAbility called on %s, HasAuthority=%d"), *GetNameSafe(GetOwner()), GetOwner()->HasAuthority());
					MulticastActivatePassiveEffect(AbilityTag, true); // Activate the ability effects
				}
				AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GetStatusTagFromSpec(*AbilitySpec));
				AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Equipped);
			}

			AssignSlotToAbility(AbilitySpec, NewSlotTag);
			
			
			if (StatusTag.MatchesTagExact(GameplayTags.Abilities_Status_Unlocked))
			{
				AbilitySpec->GetDynamicSpecSourceTags().RemoveTag(GameplayTags.Abilities_Status_Unlocked);
				AbilitySpec->GetDynamicSpecSourceTags().AddTag(GameplayTags.Abilities_Status_Equipped);
				StatusTag = GameplayTags.Abilities_Status_Equipped;
			}

			
			MarkAbilitySpecDirty(*AbilitySpec); // Force Replicate
		}
		ClientEquipAbility(AbilityTag, StatusTag, NewSlotTag, PrevSlotTag);
	}
}

void USeptentAbilitySystemComponent::ClientEquipAbility_Implementation(const FGameplayTag& AbilityTag,
	const FGameplayTag& StatusTag, const FGameplayTag& NewSlotTag, const FGameplayTag& PrevSlotTag)
{
	AbilityEquippedDelegate.Broadcast(AbilityTag, StatusTag, NewSlotTag, PrevSlotTag);
}

bool USeptentAbilitySystemComponent::GetDescriptionsByAbilityTag(const FGameplayTag& AbilityTag, FString& OutDescription,
                                                              FString& OutNextLevelDescription)
{
	if (const FGameplayAbilitySpec* AbilitySpec = GetSpecFromAbilityTag(AbilityTag))
	{
		if (USeptentGameplayAbility* SeptentGameplayAbility = Cast<USeptentGameplayAbility>(AbilitySpec->Ability.Get()))
		{
			OutDescription = SeptentGameplayAbility->GetDescription(AbilitySpec->Level);
			OutNextLevelDescription = SeptentGameplayAbility->GetNextLevelDescription(AbilitySpec->Level + 1);
			return true;
		}
	}

	UAbilityInfo* AbilityInfo = USeptentAbilitySystemLibrary::GetAbilityInfo(GetAvatarActor());
	if (!AbilityTag.IsValid() || AbilityTag.MatchesTagExact(FSeptentGameplayTags::Get().Abilities_None))
	{
		OutDescription = FString();
	} else
	{
		OutDescription = USeptentGameplayAbility::GetLockedDescription(AbilityInfo->FindAbilityInfoForTag(AbilityTag).LevelRequirement);
	}
	OutNextLevelDescription = FString();
	return false;
}

void USeptentAbilitySystemComponent::ClearSlot(FGameplayAbilitySpec* Spec)
{
	const FGameplayTag& Slot = GetInputTagFromSpec(*Spec);
	Spec->GetDynamicSpecSourceTags().RemoveTag(Slot);
}

void USeptentAbilitySystemComponent::ClearAbilitiesOfSlot(const FGameplayTag& SlotTag)
{
	FScopedAbilityListLock ActiveScopeLock(*this); // lock the list of abilities before this loop has finished
	for (FGameplayAbilitySpec& AbilitySpec : GetActivatableAbilities())
	{
		if (AbilitySpec.GetDynamicSpecSourceTags().HasTagExact(SlotTag))
		{
			ClearSlot(&AbilitySpec);
		}
	}
}

void USeptentAbilitySystemComponent::OnRep_ActivateAbilities()
{
	Super::OnRep_ActivateAbilities();


	if (!bStartupAbilitiesGiven)
	{
		bStartupAbilitiesGiven = true;
		AbilitiesGivenDelegate.Broadcast();
	}
}

void USeptentAbilitySystemComponent::ClientUpdateAbilityStatus_Implementation(const FGameplayTag& AbilityTag,
	const FGameplayTag& StatusTag, const int32 AbilityLevel)
{
	AbilityStatusChangedDelegate.Broadcast(AbilityTag, StatusTag, AbilityLevel);
}


void USeptentAbilitySystemComponent::ClientEffectApplied_Implementation(UAbilitySystemComponent* AbilitySystemComponent,
                                                                     const FGameplayEffectSpec& EffectSpec,
                                                                     FActiveGameplayEffectHandle ActiveGameplayEffectHandle)
{
	FGameplayTagContainer TagContainer;
	EffectSpec.GetAllAssetTags(TagContainer);

	EffectAssetTags.Broadcast(TagContainer);
}
