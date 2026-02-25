// ZZ


#include "UI/WidgetController/SpellMenuWidgetController.h"

#include "SeptentGameplayTags.h"
#include "AbilitySystem/SeptentAbilitySystemComponent.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/SeptentPlayerState.h"

void USpellMenuWidgetController::BroadcastInitialValues()
{
	Super::BroadcastInitialValues();
	BroadcastAbilityInfo();
	SpellPointsDelegate.Broadcast(GetSeptentPS()->GetSpellPoints());
}


void USpellMenuWidgetController::BindCallbacksToDependencies()
{
	Super::BindCallbacksToDependencies();
	GetSeptentASC()->AbilityStatusChangedDelegate.AddLambda(
		[this](const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const int32 AbilityLevel)
		{
			if (SelectedAbility.Ability.MatchesTagExact(AbilityTag)) // Handles an edge case when the level up happens while the spell menu is opened and a spell is selected
			{
				SelectedAbility.Status = StatusTag;
				bool bSpendPointsButtonEnabled = false;
				bool bEquipButtonEnabled = false;
				ShouldEnableButtons(StatusTag, CurrentSpellPoints, bSpendPointsButtonEnabled, bEquipButtonEnabled);

				FString Description;
				FString NextLevelDescription;
				GetSeptentASC()->GetDescriptionsByAbilityTag(AbilityTag, Description, NextLevelDescription);
				SpellGlobeSelectedDelegate.Broadcast(bSpendPointsButtonEnabled, bEquipButtonEnabled, Description, NextLevelDescription);
			}
			if (AbilityInfo)
			{
				FSeptentAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
				Info.StatusTag = StatusTag;
				AbilityInfoDelegate.Broadcast(Info);
			}
		}
	);

	GetSeptentASC()->AbilityEquippedDelegate.AddUObject(this, &USpellMenuWidgetController::OnAbilityEquipped);

	GetSeptentPS()->OnSpellPointsChangedDelegate.AddLambda(
		[this](const int32 NewValue)
		{
			SpellPointsDelegate.Broadcast(NewValue);
			
			CurrentSpellPoints = NewValue; // Handles an edge case when the level up happens while the spell menu is opened and a spell is selected
			bool bSpendPointsButtonEnabled = false;
			bool bEquipButtonEnabled = false;
			ShouldEnableButtons(SelectedAbility.Status, CurrentSpellPoints, bSpendPointsButtonEnabled, bEquipButtonEnabled);
			
			FString Description;
			FString NextLevelDescription;
			GetSeptentASC()->GetDescriptionsByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);
			SpellGlobeSelectedDelegate.Broadcast(bSpendPointsButtonEnabled, bEquipButtonEnabled, Description, NextLevelDescription);
		}
	);
}

void USpellMenuWidgetController::SpellGlobeSelected(const FGameplayTag& AbilityTag)
{
	if (bWaitingforEquipSelection)
	{
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingforEquipSelection = false;
	}
	
	bool bSpendPointsButtonEnabled = false;
	bool bEquipButtonEnabled = false;

	const FSeptentGameplayTags GameplayTags = FSeptentGameplayTags::Get();
	const int32 SpellPoints = GetSeptentPS()->GetSpellPoints();
	FGameplayTag AbilityStatus;

	const bool bTagValid = AbilityTag.IsValid();
	const bool bTagNone = AbilityTag.MatchesTagExact(GameplayTags.Abilities_None);
	const FGameplayAbilitySpec* AbilitySpec = GetSeptentASC()->GetSpecFromAbilityTag(AbilityTag);
	const bool bSpecValid = AbilitySpec != nullptr;
	
	if (!bTagValid || bTagNone || !bSpecValid)
	{
		AbilityStatus = GameplayTags.Abilities_Status_Locked;
	} else
	{
		AbilityStatus = GetSeptentASC()->GetStatusTagFromSpec(*AbilitySpec);
	}

	SelectedAbility.Ability = AbilityTag;
	SelectedAbility.Status = AbilityStatus;
	
	ShouldEnableButtons(AbilityStatus, SpellPoints, bSpendPointsButtonEnabled, bEquipButtonEnabled);

	FString Description;
	FString NextLevelDescription;
	GetSeptentASC()->GetDescriptionsByAbilityTag(SelectedAbility.Ability, Description, NextLevelDescription);
	SpellGlobeSelectedDelegate.Broadcast(bSpendPointsButtonEnabled, bEquipButtonEnabled, Description, NextLevelDescription);
}

void USpellMenuWidgetController::SpendPointButtonClicked()
{
	GetSeptentASC()->ServerSpendSpellPoint(SelectedAbility.Ability);
}

void USpellMenuWidgetController::GlobeDeselect()
{
	if (bWaitingforEquipSelection)
	{
		const FGameplayTag SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
		StopWaitingForEquipDelegate.Broadcast(SelectedAbilityType);
		bWaitingforEquipSelection = false;
	}
	
	SelectedAbility.Ability = FSeptentGameplayTags::Get().Abilities_None;
	SelectedAbility.Status = FSeptentGameplayTags::Get().Abilities_Status_Locked;
	SpellGlobeSelectedDelegate.Broadcast(false, false, FString(), FString());
}

void USpellMenuWidgetController::EquipButtonPressed()
{
	const FGameplayTag AbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;

	WaitForEquipDelegate.Broadcast(AbilityType);
	bWaitingforEquipSelection = true;

	const FGameplayTag SelectedStatus = GetSeptentASC()->GetStatusTagFromAbilityTag(SelectedAbility.Ability);
	if (SelectedStatus.MatchesTagExact(FSeptentGameplayTags::Get().Abilities_Status_Equipped))
	{
		SelectedSlot = GetSeptentASC()->GetInputTagFromAbilityTag(SelectedAbility.Ability);
	}
}

void USpellMenuWidgetController::SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType)
{
	if (!bWaitingforEquipSelection) return;
	// Check selected ability against slot's ability type. (don't equip passive to offensive)
	const FGameplayTag& SelectedAbilityType = AbilityInfo->FindAbilityInfoForTag(SelectedAbility.Ability).AbilityType;
	if (!SelectedAbilityType.MatchesTagExact(AbilityType)) return;

	GetSeptentASC()->ServerEquipAbility(SelectedAbility.Ability, SlotTag);
}

void USpellMenuWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag,
	const FGameplayTag& NewSlotTag, const FGameplayTag& PrevSlotTag)
{
	bWaitingforEquipSelection = false;

	const FSeptentGameplayTags GameplayTags = FSeptentGameplayTags::Get();

	FSeptentAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = FSeptentGameplayTags::Get().Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PrevSlotTag;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	// Broadcast empty info if PrevSlot is a valid slot. Only if equipping an already-equipped spell
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	FSeptentAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = StatusTag;
	Info.InputTag = NewSlotTag;
	AbilityInfoDelegate.Broadcast(Info);

	StopWaitingForEquipDelegate.Broadcast(AbilityInfo->FindAbilityInfoForTag(AbilityTag).AbilityType);
	SpellGlobeReassignedDelegate.Broadcast(AbilityTag);
	GlobeDeselect();
}

void USpellMenuWidgetController::ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints,
                                                     bool& bSpendPointsButtonEnabled, bool& bEquipButtonEnabled)
{
	const FSeptentGameplayTags GameplayTags = FSeptentGameplayTags::Get();
	
	bSpendPointsButtonEnabled = false;
	bEquipButtonEnabled = false;

	if (AbilityStatus.MatchesTagExact(FSeptentGameplayTags::Get().Abilities_Status_Equipped))
	{
		bEquipButtonEnabled = true;
		if (SpellPoints > 0)
		{
			bSpendPointsButtonEnabled = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(FSeptentGameplayTags::Get().Abilities_Status_Eligible))
	{
		if (SpellPoints > 0)
		{
			bSpendPointsButtonEnabled = true;
		}
	}
	else if (AbilityStatus.MatchesTagExact(FSeptentGameplayTags::Get().Abilities_Status_Unlocked))
	{
		bEquipButtonEnabled = true;
		if (SpellPoints > 0)
		{
			bSpendPointsButtonEnabled = true;
		}
	}
}

