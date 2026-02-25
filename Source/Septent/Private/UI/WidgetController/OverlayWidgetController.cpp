// ZZ


#include "UI/WidgetController/OverlayWidgetController.h"

#include "SeptentGameplayTags.h"
#include "AbilitySystem/SeptentAbilitySystemComponent.h"
#include "AbilitySystem/SeptentAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "Player/SeptentPlayerState.h"

void UOverlayWidgetController::BroadcastInitialValues()
{
	
	OnHealthChanged.Broadcast(GetSeptentAS()->GetHealth());
	OnMaxHealthChanged.Broadcast(GetSeptentAS()->GetMaxHealth());
	OnManaChanged.Broadcast(GetSeptentAS()->GetMana());
	OnMaxManaChanged.Broadcast(GetSeptentAS()->GetMaxMana());
}

void UOverlayWidgetController::BindCallbacksToDependencies()
{
	GetSeptentPS()->OnXPChangedDelegate.AddUObject(this, &UOverlayWidgetController::OnXPChanged);

	GetSeptentPS()->OnLevelChangedDelegate.AddLambda(
		[this](int32 NewLevel, bool bLevelUp)
		{
			OnPlayerLevelChangedDelegate.Broadcast(NewLevel, bLevelUp);
		}
	);
	// AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	// 	SeptentAttributeSet->GetHealthAttribute()).AddUObject(this, &UOverlayWidgetController::HealthChanged);
	//
	// AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	// 	SeptentAttributeSet->GetMaxHealthAttribute()).AddUObject(this, &UOverlayWidgetController::MaxHealthChanged);
	//
	// AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	// 	SeptentAttributeSet->GetManaAttribute()).AddUObject(this, &UOverlayWidgetController::ManaChanged);
	//
	// AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
	// 	SeptentAttributeSet->GetMaxManaAttribute()).AddUObject(this, &UOverlayWidgetController::MaxManaChanged);

	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetSeptentAS()->GetHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnHealthChanged.Broadcast(Data.NewValue);
		});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetSeptentAS()->GetMaxHealthAttribute()).AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnMaxHealthChanged.Broadcast(Data.NewValue);
		});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetSeptentAS()->GetManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnManaChanged.Broadcast(Data.NewValue);
		});
	AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(
		GetSeptentAS()->GetMaxManaAttribute()).AddLambda([this](const FOnAttributeChangeData& Data)
		{
			OnMaxManaChanged.Broadcast(Data.NewValue);
		});

	if (GetSeptentASC())
	{
		if (GetSeptentASC()->bStartupAbilitiesGiven)
		{
			BroadcastAbilityInfo();
		}
		else
		{
			GetSeptentASC()->AbilitiesGivenDelegate.AddUObject(this, &UOverlayWidgetController::BroadcastAbilityInfo);
		}
		
		GetSeptentASC()->EffectAssetTags.AddLambda(
			[this](const FGameplayTagContainer& AssetTags)
			{
				for (const FGameplayTag& Tag : AssetTags)
				{
					FGameplayTag MessageTag = FGameplayTag::RequestGameplayTag(TEXT("Message"));
					if (Tag.MatchesTag(MessageTag))
					{
						const FUIWidgetRow* Row = GetDataTableRowByTag<FUIWidgetRow>(MessageWidgetDataTable, Tag);
						MessageWidgetRowDelegate.Broadcast(*Row);
					}
				}
			}
		);

		GetSeptentASC()->AbilityEquippedDelegate.AddUObject(this, &UOverlayWidgetController::OnAbilityEquipped);
	}

	
}

void UOverlayWidgetController::OnXPChanged(const int32 NewXP)
{
	ULevelUpInfo* LevelUpInfo = GetSeptentPS()->LevelUpInfo;
	checkf(LevelUpInfo, TEXT("Unable to find LevelUpInfo. Please fill out SeptentPlayerState Blueprint"));

	const int32 Level = LevelUpInfo->FindLevelForXP(NewXP);

	if (const int32 MaxLevel = LevelUpInfo->LevelUpInformation.Num(); Level <= MaxLevel && Level > 0)
	{
		const int32 LevelUpRequirement = LevelUpInfo->LevelUpInformation[Level].LevelUpRequirement;
		const int32 PreviousLevelUpRequirement = LevelUpInfo->LevelUpInformation[Level - 1].LevelUpRequirement;

		const int32 DeltaLevelRequirement = LevelUpRequirement - PreviousLevelUpRequirement;
		const int32 XPForThisLevel = NewXP - PreviousLevelUpRequirement;

		const float XPBarPercent = static_cast<float>(XPForThisLevel) / static_cast<float>(DeltaLevelRequirement);

		OnXPPercentChangedDelegate.Broadcast(XPBarPercent);
	}
}

void UOverlayWidgetController::OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag,
	const FGameplayTag& NewSlotTag, const FGameplayTag& PrevSlotTag) const
{
	const FSeptentGameplayTags GameplayTags = FSeptentGameplayTags::Get();

	FSeptentAbilityInfo LastSlotInfo;
	LastSlotInfo.StatusTag = FSeptentGameplayTags::Get().Abilities_Status_Unlocked;
	LastSlotInfo.InputTag = PrevSlotTag;
	LastSlotInfo.CooldownTag = FGameplayTag::EmptyTag;
	LastSlotInfo.AbilityTag = GameplayTags.Abilities_None;
	// Broadcast empty info if PrevSlot is a valid slot. Only if equipping an already-equipped spell
	AbilityInfoDelegate.Broadcast(LastSlotInfo);

	FSeptentAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);
	Info.StatusTag = StatusTag;
	Info.InputTag = NewSlotTag;
	AbilityInfoDelegate.Broadcast(Info);
}

// void UOverlayWidgetController::HealthChanged(const FOnAttributeChangeData& Data) const
// {
// 	OnHealthChanged.Broadcast(Data.NewValue);
// }
//
// void UOverlayWidgetController::MaxHealthChanged(const FOnAttributeChangeData& Data) const
// {
// 	OnMaxHealthChanged.Broadcast(Data.NewValue);
// }
//
// void UOverlayWidgetController::ManaChanged(const FOnAttributeChangeData& Data) const
// {
// 	OnManaChanged.Broadcast(Data.NewValue);
// }
//
// void UOverlayWidgetController::MaxManaChanged(const FOnAttributeChangeData& Data) const
// {
// 	OnMaxManaChanged.Broadcast(Data.NewValue);
// }
