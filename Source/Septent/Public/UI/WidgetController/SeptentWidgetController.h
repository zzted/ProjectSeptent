// ZZ

#pragma once

#include "CoreMinimal.h"
#include "UObject/NoExportTypes.h"
#include "SeptentWidgetController.generated.h"

class UAbilityInfo;
class USeptentAttributeSet;
class USeptentAbilitySystemComponent;
class ASeptentPlayerState;
class ASeptentPlayerController;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnStatChangedSignature, int32, NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FAbilityInfoSignature, const FSeptentAbilityInfo&, Info);


class UAttributeSet;
class UAbilitySystemComponent;

USTRUCT(BlueprintType)
struct FWidgetControllerParams
{
	GENERATED_BODY();

	FWidgetControllerParams() {}

	FWidgetControllerParams(APlayerController* InPlayerController, APlayerState* InPlayerState,
		UAbilitySystemComponent* InAbilitySystemComponent, UAttributeSet* InAttributeSet)
	: PlayerController(InPlayerController), PlayerState(InPlayerState),
	AbilitySystemComponent(InAbilitySystemComponent), AttributeSet(InAttributeSet)
	{}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerController> PlayerController = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<APlayerState> PlayerState = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent = nullptr;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UAttributeSet> AttributeSet = nullptr;
};

/**
 * @class USeptentWidgetController
 * @brief The USeptentWidgetController class serves as a base class for controlling widgets in the Septent system.
 *
 * This class provides functionality to manage widget behavior, broadcast initial values, and bind callbacks
 * to relevant gameplay dependencies. It maintains references to important gameplay components that affect the widget.
 */
UCLASS()
class SEPTENT_API USeptentWidgetController : public UObject
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SetWidgetControllerParams(const FWidgetControllerParams& WidgetControllerParams);

	UFUNCTION(BlueprintCallable)
	virtual void BroadcastInitialValues();
	virtual void BindCallbacksToDependencies();

	UPROPERTY(BlueprintAssignable, Category = "GAS|Messages")
	FAbilityInfoSignature AbilityInfoDelegate; // Listened by all ability rows for ability changes

	void BroadcastAbilityInfo();
	
protected:
	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerController> PlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<APlayerState> PlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<UAttributeSet> AttributeSet;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<ASeptentPlayerController> SeptentPlayerController;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<ASeptentPlayerState> SeptentPlayerState;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<USeptentAbilitySystemComponent> SeptentAbilitySystemComponent;

	UPROPERTY(BlueprintReadOnly, Category = "WidgetController")
	TObjectPtr<USeptentAttributeSet> SeptentAttributeSet;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Widget Data")
	TObjectPtr<UAbilityInfo> AbilityInfo;

	ASeptentPlayerController* GetSeptentPC();
	ASeptentPlayerState* GetSeptentPS();
	USeptentAbilitySystemComponent* GetSeptentASC();
	USeptentAttributeSet* GetSeptentAS();

};
