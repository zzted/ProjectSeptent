// ZZ


#include "AbilitySystem/Abilities/DamageGameplayAbility.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"

void UDamageGameplayAbility::CauseDamage(AActor* TargetActor)
{
	FGameplayEffectSpecHandle DamageSpecHandle = MakeOutgoingGameplayEffectSpec(DamageEffectClass, 1);
	// for (TTuple<FGameplayTag, FScalableFloat> Pair : DamageTypes)
	// {
	// 	const FGameplayTag& GameplayTag = Pair.Key;
	// 	const FScalableFloat& DamageCurve = Pair.Value;
	// 	const float ScaledDamage = DamageCurve.GetValueAtLevel(GetAbilityLevel());
	// 	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, GameplayTag, ScaledDamage);
	// }

	const float ScaledDamage = DamageAmount.GetValueAtLevel(GetAbilityLevel());
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(DamageSpecHandle, DamageTypeTag, ScaledDamage);
	GetAbilitySystemComponentFromActorInfo()->ApplyGameplayEffectSpecToTarget(*DamageSpecHandle.Data.Get(), UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor));
}

FDamageEffectParams UDamageGameplayAbility::MakeDamageEffectParamsFromClassDefaults(AActor* TargetActor) const
{
	FDamageEffectParams Params;
	Params.WorldContextObject = GetAvatarActorFromActorInfo();
	Params.DamageGameplayEffectClass = DamageEffectClass;
	Params.SourceAbilitySystemComponent = GetAbilitySystemComponentFromActorInfo();
	Params.TargetAbilitySystemComponent = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(TargetActor);
	Params.BaseDamage = DamageAmount.GetValueAtLevel(GetAbilityLevel());
	Params.AbilityLevel = GetAbilityLevel();
	Params.DamageTypeTag = DamageTypeTag;
	Params.DebuffChance = DebuffChance;
	Params.DebuffDuration = DebuffDuration;
	Params.DebuffDamage = DebuffDamage;
	Params.DebuffFrequency = DebuffFrequency;
	Params.DeathImpulseMagnitude = DeathImpulseMagnitude;
	Params.KnockbackForceMagnitude = KnockbackForceMagnitude;
	Params.KnockbackChance = KnockbackChance;
	if (IsValid(TargetActor)) // Setting default value for knockbacks for MeleeAttack etc. Firebolt has these values overridden in Projectiles.cpp
	{
		FRotator Rotation = (TargetActor->GetActorLocation() - GetAvatarActorFromActorInfo()->GetActorLocation()).Rotation();
		Rotation.Pitch = 45.f;
		const FVector KnockbackDirection = Rotation.Vector();
		Params.DeathImpulse = KnockbackDirection * DeathImpulseMagnitude;
		Params.KnockbackForce = KnockbackDirection * KnockbackForceMagnitude;
	}
	if (bIsRadialDamage)
	{
		Params.bIsRadialDamage = bIsRadialDamage;
		Params.RadialDamageInnerRadius = RadialDamageInnerRadius;
		Params.RadialDamageOuterRadius = RadialDamageOuterRadius;
	}
	return Params;
}

float UDamageGameplayAbility::GetDamageAtLevel() const
{
	return DamageAmount.GetValueAtLevel(GetAbilityLevel());
}

FTaggedMontage UDamageGameplayAbility::GetRandomTaggedMontageFromArray(const TArray<FTaggedMontage>& TaggedMontages) const
{
	if (TaggedMontages.Num() == 0) return FTaggedMontage();
	return TaggedMontages[FMath::RandRange(0, TaggedMontages.Num() - 1)];
}

// float UDamageGameplayAbility::GetDamageByDamageType(float InLevel, const FGameplayTag& DamageType) const
// {
// 	checkf(DamageTypes.Contains(DamageType), TEXT("GameplayAbility [%s] does not have a DamageType [%s]"), *GetNameSafe(this), *DamageType.ToString());
// 	return DamageTypes[DamageType].GetValueAtLevel(InLevel);
// }
