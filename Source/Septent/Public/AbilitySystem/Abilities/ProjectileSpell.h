// ZZ

#pragma once

#include "CoreMinimal.h"
#include "DamageGameplayAbility.h"
#include "ProjectileSpell.generated.h"

class AProjectile;
/**
 * 
 */
UCLASS()
class SEPTENT_API UProjectileSpell : public UDamageGameplayAbility
{
	GENERATED_BODY()



protected:
	virtual void ActivateAbility(const FGameplayAbilitySpecHandle Handle, const FGameplayAbilityActorInfo* ActorInfo,
		const FGameplayAbilityActivationInfo ActivationInfo, const FGameplayEventData* TriggerEventData) override;

	UFUNCTION(BlueprintCallable, Category = "Projectile")
	void SpawnProjectile(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag, const bool bOverridePitch = false, const float OverridePitch = 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<AProjectile> ProjectileClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	int32 NumProjectiles = 5;
};
 