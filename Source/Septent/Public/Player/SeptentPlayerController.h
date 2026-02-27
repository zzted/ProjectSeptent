// ZZ

#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "GameFramework/PlayerController.h"
#include "SeptentPlayerController.generated.h"

class UInv_InventoryComponent;
class IHighlightInterface;
class AMagicCircle;
class UNiagaraSystem;
class UDamageTextComponent;
class USeptentInputConfig;
class UInputMappingContext;
class UInputAction;
struct FInputActionValue;
class USeptentAbilitySystemComponent;
class USplineComponent;

enum class ETargetingStatus : uint8
{
	TargetingEnemy,
	TargetingNonenemy,
	NotTargeting
};

/**
 * 
 */
UCLASS()
class SEPTENT_API ASeptentPlayerController : public APlayerController
{
	GENERATED_BODY()
public:
	ASeptentPlayerController();
	virtual void PlayerTick(float DeltaTime) override;
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	void ToggleInventory();

	UFUNCTION(Client, Reliable)
	void ShowDamageNumber(const float DamageAmount, ACharacter* TargetCharacter, const bool bBlockedHit, const bool bCriticalHit);

	UFUNCTION(BlueprintCallable)
	void ShowMagicCircle(UMaterialInterface* DecalMaterial = nullptr);

	UFUNCTION(BlueprintCallable)
	void HideMagicCircle();
	
	
protected:
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;
private:
	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputMappingContext> SeptentInputMappingContext;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditAnywhere, Category = "Input")
	TObjectPtr<UInputAction> ShiftAction;
	
	UPROPERTY(EditDefaultsOnly, Category="Input|Inventory")
	TObjectPtr<UInputAction> ToggleInventoryAction;
	
	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent; // Added in BP

	void ShiftPressed() {bShiftPressed = true;};
	void ShiftReleased() {bShiftPressed = false;};
	bool bShiftPressed = false;

	void Move(const FInputActionValue& InputActionValue);

	void CursorTrace();
	FHitResult CursorHit;
	TObjectPtr<AActor> LastActor;
	TObjectPtr<AActor> ThisActor;

	static void HighLightActor(AActor* InActor);
	static void UnHighLightActor(AActor* InActor);

	void AbilityInputTagPressed(FGameplayTag InputTag);
	void AbilityInputTagReleased(FGameplayTag InputTag);
	void AbilityInputTagHeld(FGameplayTag InputTag);

	UPROPERTY(EditDefaultsOnly, Category = "Input")
	TObjectPtr<USeptentInputConfig> InputConfig;

	UPROPERTY()
	TObjectPtr<USeptentAbilitySystemComponent> SeptentAbilitySystemComponent;

	USeptentAbilitySystemComponent* GetAbilitySystemComponent();

	/*
	 *  click to run related
	 */
	FVector CachedDestination = FVector::ZeroVector;
	float FollowTime = 0.f;
	float ShortPressThreshold = 0.5f;
	bool bAutoRunning = false;
	ETargetingStatus TargetingStatus = ETargetingStatus::NotTargeting;

	UPROPERTY(EditDefaultsOnly)
	float AutoRunAcceptanceRadius = 50.f;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USplineComponent> Spline;

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<UNiagaraSystem> ClickNiagaraSystem;

	void AutoRun();

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UDamageTextComponent> DamageTextComponentClass;

	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<AMagicCircle> MagicCircleClass;

	UPROPERTY()
	TObjectPtr<AMagicCircle> MagicCircle;

	void UpdateMagicCircleLocation() const;
};
