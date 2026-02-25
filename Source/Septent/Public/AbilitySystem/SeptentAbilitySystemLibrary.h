// ZZ

#pragma once

#include "CoreMinimal.h"
#include "SeptentAbilitySystemTypes.h"
#include "GameplayEffectTypes.h"
#include "Data/CharacterClassInfo.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "UI/HUD/SeptentHUD.h"
#include "SeptentAbilitySystemLibrary.generated.h"

class ULootTiers;
class ULoadScreenSaveGame;
class UAbilityInfo;
class USpellMenuWidgetController;
class UAbilitySystemComponent;
class UAttributeMenuWidgetController;
class UOverlayWidgetController;
struct FWidgetControllerParams;
/**
 * 
 */
UCLASS()
class SEPTENT_API USeptentAbilitySystemLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/*
	 * Widget Controller
	 */

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|WidgetController", meta = ( DefaultToSelf = "WorldContextObject" ))
	static bool GetWidgetControllerParams(const UObject* WorldContextObject, FWidgetControllerParams& OutWCParams, ASeptentHUD*& OutSeptentHUD);
	
	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|WidgetController", meta = ( DefaultToSelf = "WorldContextObject" ))
	static UOverlayWidgetController* GetOverlayWidgetController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|WidgetController", meta = ( DefaultToSelf = "WorldContextObject" ))
	static UAttributeMenuWidgetController* GetAttributeMenuWidgetController(const UObject* WorldContextObject);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|WidgetController", meta = ( DefaultToSelf = "WorldContextObject" ))
	static USpellMenuWidgetController* GetSpellMenuWidgetController(const UObject* WorldContextObject);

	/*
	 * Ability System Class Defaults
	 */

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAttributes(const UObject* WorldContextObject, ECharacterClass CharacterClass, float Level, UAbilitySystemComponent* AbilitySystemComponent);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|CharacterClassDefaults")
	static void InitializeDefaultAttributesFromSaveData(const UObject* WorldContextObject, UAbilitySystemComponent* AbilitySystemComponent, ULoadScreenSaveGame* SaveGame);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|CharacterClassDefaults")
	static void GiveStartupAbilities(const UObject* WorldContextObject,UAbilitySystemComponent* AbilitySystemComponent, ECharacterClass CharacterClass);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|CharacterClassDefaults")
	static UCharacterClassInfo* GetCharacterClassInfo(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|CharacterClassDefaults", meta = ( DefaultToSelf = "WorldContextObject" ))
	static UAbilityInfo* GetAbilityInfo(const UObject* WorldContextObject);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|CharacterClassDefaults", meta = ( DefaultToSelf = "WorldContextObject" ))
	static ULootTiers* GetLootTiers(const UObject* WorldContextObject);

	/*
	 * Effect Context Getters
	 */
	
	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static bool IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static bool IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static bool IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static float GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static float GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static float GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static FGameplayTag GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static FVector GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static FVector GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static bool IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static float GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static float GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static FVector GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle);
	

	/*
	 * Effect Context Setters
	 */

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetIsSuccessfulDebuff(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsSuccessfulDebuff);
	// UPARAM(ref) tries to make the non-constant ref parameter an input node, otherwise UE make it and output node
	
	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetDebuffDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const float InDamage);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetDebuffDuration(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const float InDuration);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetDebuffFrequency(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const float InFrequency);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetDamageType(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FGameplayTag& InDamageType);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetDeathImpulse(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InImpulse);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetKnockbackForce(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InForce);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetIsBlockedHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsBlockedHit);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetIsCriticalHit(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsCriticalHit);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetIsRadialDamage(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, bool bInIsRadialDamage);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetRadialDamageInnerRadius(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InRadialDamageInnerRadius);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetRadialDamageOuterRadius(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, float InRadialDamageOuterRadius);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetRadialDamageOrigin(UPARAM(ref) FGameplayEffectContextHandle& EffectContextHandle, const FVector& InRadialDamageOrigin);

	/*
	 * Gameplay Mechanics
	 */

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayMechanics")
	static void GetLivePlayersWithinRadius(const UObject* WorldContextObject, TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, const float Radius, const FVector& SphereOrigin);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayMechanics")
	static void GetClosestTargets(const int32 MaxTargets, const TArray<AActor*>& InTargets, TArray<AActor*>& OutClosestTargets, const FVector& Origin);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayMechanics")
	static bool IsNotFriend(const AActor* FirstActor, const AActor* SecondActor);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static FGameplayEffectContextHandle ApplyDamageEffectsFromDamageEffectParams(const FDamageEffectParams& DamageEffectParams);

	static int32 GetXPRewardForClassAndLevel(const UObject* WorldContextObject, const ECharacterClass CharacterClass, const int32 Level);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayMechanics")
	static TArray<FRotator> EvenlySpacedRotators(const FVector& Forward, const FVector& Axis, float Spread, int32 NumRotators);

	UFUNCTION(BlueprintPure, Category = "SeptentAbilitySystemLibrary|GameplayMechanics")
	static TArray<FVector> EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis, float Spread, int32 NumVectors);

	/*
	 * Damage Effect Params
	 */

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetIsRadialDamageEffectParam(UPARAM(ref) FDamageEffectParams& DamageEffectParams, bool bInIsRadialDamage, float InnerRadius, float OuterRadius, FVector Origin);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetKnockbackDirection(UPARAM(ref) FDamageEffectParams& DamageEffectParams, UPARAM(ref) FVector& KnockbackDirection, float Magnitude = 0.f);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetDeathImpulseDirection(UPARAM(ref) FDamageEffectParams& DamageEffectParams, UPARAM(ref) FVector& DeathImpulseDirection, float Magnitude = 0.f);

	UFUNCTION(BlueprintCallable, Category = "SeptentAbilitySystemLibrary|GameplayEffect")
	static void SetTargetEffectParamsASC(UPARAM(ref) FDamageEffectParams& DamageEffectParams, UAbilitySystemComponent* InASC);
};
