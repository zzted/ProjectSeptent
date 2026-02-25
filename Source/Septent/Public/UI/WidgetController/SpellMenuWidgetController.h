// ZZ

#pragma once

#include "CoreMinimal.h"
#include "SeptentGameplayTags.h"
#include "GameplayTagContainer.h"
#include "UI/WidgetController/SeptentWidgetController.h"
#include "SpellMenuWidgetController.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FSpellGlobeSelectedSignature, bool, bSpendPointsButtonEnabled, bool, bEquipButtonEnabled, FString, DescriptionString, FString, NextLevelDescriptionString);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWaitForEquipSelectionSignature, const FGameplayTag&, AbilityType);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSpellGlobeReassignedSignature, const FGameplayTag&, AbilityType);

struct FSelectedAbility
{
	FGameplayTag Ability = FGameplayTag();
	FGameplayTag Status = FGameplayTag();
};

struct FGameplayTag;
/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SEPTENT_API USpellMenuWidgetController : public USeptentWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;
	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FSpellGlobeSelectedSignature SpellGlobeSelectedDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnStatChangedSignature SpellPointsDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FWaitForEquipSelectionSignature WaitForEquipDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FWaitForEquipSelectionSignature StopWaitingForEquipDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FSpellGlobeReassignedSignature SpellGlobeReassignedDelegate;

	UFUNCTION(BlueprintCallable)
	void SpellGlobeSelected(const FGameplayTag& AbilityTag);

	UFUNCTION(BlueprintCallable)
	void SpendPointButtonClicked();

	UFUNCTION(BlueprintCallable)
	void GlobeDeselect();

	UFUNCTION(BlueprintCallable)
	void EquipButtonPressed();

	UFUNCTION(BlueprintCallable)
	void SpellRowGlobePressed(const FGameplayTag& SlotTag, const FGameplayTag& AbilityType);

	void OnAbilityEquipped(const FGameplayTag& AbilityTag, const FGameplayTag& StatusTag, const FGameplayTag& NewSlotTag, const FGameplayTag& PrevSlotTag);

private:
	static void ShouldEnableButtons(const FGameplayTag& AbilityStatus, int32 SpellPoints, bool& bSpendPointsButtonEnabled, bool& bEquipButtonEnabled);

	FSelectedAbility SelectedAbility = {FSeptentGameplayTags::Get().Abilities_None, FSeptentGameplayTags::Get().Abilities_Status_Locked};
	int32 CurrentSpellPoints = 0;
	bool bWaitingforEquipSelection = false;

	FGameplayTag SelectedSlot;
};
