// ZZ


#include "UI/WidgetController/AttributeMenuWidgetController.h"

#include "SeptentGameplayTags.h"
#include "AbilitySystem/SeptentAbilitySystemComponent.h"
#include "AbilitySystem/SeptentAttributeSet.h"
#include "Player/SeptentPlayerState.h"

void UAttributeMenuWidgetController::BroadcastInitialValues()
{
	check(AttributeInfo);;

	for (auto& Pair : GetSeptentAS()->TagsToAttributes)
	{
		// Deprecated delegate method
		// Info.AttributeValue = Pair.Value.Execute().GetNumericValue(SeptentAttributeSet);
		BroadcastAttributeInfo(Pair.Key, Pair.Value());
	}
	
	AttributePointsDelegate.Broadcast(GetSeptentPS()->GetAttributePoints());
	SpellPointsDelegate.Broadcast(GetSeptentPS()->GetSpellPoints());
}

void UAttributeMenuWidgetController::BindCallbacksToDependencies()
{
	for (auto& Pair : GetSeptentAS()->TagsToAttributes)
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(Pair.Value()).AddLambda(
			[this, Pair](const FOnAttributeChangeData& Data)
			{
				BroadcastAttributeInfo(Pair.Key, Pair.Value());
			}
		);
	}
	
	GetSeptentPS()->OnAttributePointsChangedDelegate.AddLambda(
	[this](const int32 NewValue)
	{
		AttributePointsDelegate.Broadcast(NewValue);
	}
);

	GetSeptentPS()->OnSpellPointsChangedDelegate.AddLambda(
		[this](const int32 NewValue)
		{
			SpellPointsDelegate.Broadcast(NewValue);
		}
	);
}

void UAttributeMenuWidgetController::UpgradeAttribute(const FGameplayTag& AttributeTag)
{
	GetSeptentASC()->UpgradeAttribute(AttributeTag);
}

void UAttributeMenuWidgetController::BroadcastAttributeInfo(const FGameplayTag& AttributeTag,
                                                            const FGameplayAttribute& Attribute) const
{
	FSeptentAttributeInfo Info = AttributeInfo->FindAttributeInfoForTag(AttributeTag);
	Info.AttributeValue = Attribute.GetNumericValue(AttributeSet);
	AttributeInfoDelegate.Broadcast(Info);
}
