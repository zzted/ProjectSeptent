// ZZ

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "SeptentHUD.generated.h"

class USpellMenuWidgetController;
class UAttributeMenuWidgetController;
class UAbilitySystemComponent;
class UAttributeSet;
struct FWidgetControllerParams;
class UOverlayWidgetController;
class USeptentUserWidget;
/**
 * @class ASeptentHUD
 * @brief A custom Heads-Up Display (HUD) class for the Septent game, responsible for managing the overlay UI and its related components.
 *
 * Manages user interface elements, including the overlay widget and its associated widget controllers, to provide
 * a seamless interface tied to the player's state, abilities, and attributes.
 */
UCLASS()
class SEPTENT_API ASeptentHUD : public AHUD
{
	GENERATED_BODY()
	
public:

	

	UOverlayWidgetController* GetOverlayWidgetController(const FWidgetControllerParams& WidgetControllerParams);

	UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const FWidgetControllerParams& WidgetControllerParams);

	USpellMenuWidgetController* GetSpellMenuWidgetController(const FWidgetControllerParams& WidgetControllerParams);

	void InitOverlay(APlayerController* PlayerController, APlayerState* PlayerState,
		UAbilitySystemComponent* AbilitySystemComponent, UAttributeSet* AttributeSet);

protected:

private:
	UPROPERTY()
	TObjectPtr<USeptentUserWidget> OverlayWidget;
	
	UPROPERTY(EditAnywhere)
	TSubclassOf<USeptentUserWidget> OverlayWidgetClass;

	UPROPERTY()
	TObjectPtr<UOverlayWidgetController> OverlayWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UOverlayWidgetController> OverlayWidgetControllerClass;

	UPROPERTY()
	TObjectPtr<UAttributeMenuWidgetController> AttributeMenuWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UAttributeMenuWidgetController> AttributeMenuWidgetControllerClass;

	UPROPERTY()
	TObjectPtr<USpellMenuWidgetController> SpellMenuWidgetController;

	UPROPERTY(EditAnywhere)
	TSubclassOf<USpellMenuWidgetController> SpellMenuWidgetControllerClass;
};
