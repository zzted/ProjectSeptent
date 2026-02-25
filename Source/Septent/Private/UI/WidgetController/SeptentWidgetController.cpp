// ZZ


#include "UI/WidgetController/SeptentWidgetController.h"

#include "AbilitySystem/SeptentAbilitySystemComponent.h"
#include "AbilitySystem/SeptentAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "Player/SeptentPlayerController.h"
#include "Player/SeptentPlayerState.h"

void USeptentWidgetController::SetWidgetControllerParams(const FWidgetControllerParams& WidgetControllerParams)
{
	PlayerController = WidgetControllerParams.PlayerController;
	PlayerState = WidgetControllerParams.PlayerState;
	AbilitySystemComponent = WidgetControllerParams.AbilitySystemComponent;
	AttributeSet = WidgetControllerParams.AttributeSet;
}

void USeptentWidgetController::BroadcastInitialValues()
{
}

void USeptentWidgetController::BindCallbacksToDependencies()
{
}

void USeptentWidgetController::BroadcastAbilityInfo()
{
	if (!GetSeptentASC()->bStartupAbilitiesGiven) return;

	FForEachAbility BroadCastDelegate;
	BroadCastDelegate.BindLambda([this](const FGameplayAbilitySpec& AbilitySpec)
	{
		FSeptentAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(SeptentAbilitySystemComponent->GetAbilityTagFromSpec(AbilitySpec));
		Info.InputTag = SeptentAbilitySystemComponent->GetInputTagFromSpec(AbilitySpec);
		Info.StatusTag = SeptentAbilitySystemComponent->GetStatusTagFromSpec(AbilitySpec);
		AbilityInfoDelegate.Broadcast(Info);
	});

	SeptentAbilitySystemComponent->ForEachAbility(BroadCastDelegate);
}

ASeptentPlayerController* USeptentWidgetController::GetSeptentPC()
{
	if (SeptentPlayerController == nullptr)
	{
		SeptentPlayerController = Cast<ASeptentPlayerController>(PlayerController);
	}
	return SeptentPlayerController;
}

ASeptentPlayerState* USeptentWidgetController::GetSeptentPS()
{
	if (SeptentPlayerState == nullptr)
	{
		SeptentPlayerState = Cast<ASeptentPlayerState>(PlayerState);
	}
	return SeptentPlayerState;
}

USeptentAbilitySystemComponent* USeptentWidgetController::GetSeptentASC()
{
	if (SeptentAbilitySystemComponent == nullptr)
	{
		SeptentAbilitySystemComponent = Cast<USeptentAbilitySystemComponent>(AbilitySystemComponent);
	}
	return SeptentAbilitySystemComponent;
}

USeptentAttributeSet* USeptentWidgetController::GetSeptentAS()
{
	if (SeptentAttributeSet == nullptr)
	{
		SeptentAttributeSet = Cast<USeptentAttributeSet>(AttributeSet);
	}
	return SeptentAttributeSet;
}
