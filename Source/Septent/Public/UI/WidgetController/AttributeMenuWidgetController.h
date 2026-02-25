// ZZ

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/AttributeInfo.h"
#include "UI/WidgetController/SeptentWidgetController.h"
#include "AttributeMenuWidgetController.generated.h"

struct FGameplayAttribute;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAttributeInfoSignature, const FSeptentAttributeInfo&, Info);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPlayerStatusInfoSignature, const int32, Value);

/**
 * 
 */
UCLASS(Blueprintable, BlueprintType)
class SEPTENT_API UAttributeMenuWidgetController : public USeptentWidgetController
{
	GENERATED_BODY()

public:
	virtual void BroadcastInitialValues() override;

	virtual void BindCallbacksToDependencies() override;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FAttributeInfoSignature AttributeInfoDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnStatChangedSignature AttributePointsDelegate;

	UPROPERTY(BlueprintAssignable, Category = "GAS|Attributes")
	FOnStatChangedSignature SpellPointsDelegate;

	UFUNCTION(BlueprintCallable, Category = "GAS|Attributes")
	void UpgradeAttribute(const FGameplayTag& AttributeTag);

protected:

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UAttributeInfo> AttributeInfo;

private:
	void BroadcastAttributeInfo(const FGameplayTag& AttributeTag, const FGameplayAttribute& Attribute) const;
};
