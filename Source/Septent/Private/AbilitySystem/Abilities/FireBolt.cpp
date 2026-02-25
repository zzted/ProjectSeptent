// ZZ


#include "AbilitySystem/Abilities/FireBolt.h"

#include "AbilitySystem/SeptentAbilitySystemComponent.h"
#include "AbilitySystem/SeptentAbilitySystemLibrary.h"
#include "Actor/Projectile.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/KismetSystemLibrary.h"

FString UFireBolt::GetDescription(int32 Level)
{
	// const int32 Damage = GetDamageByDamageType(Level, FSeptentGameplayTags::Get().Damage_Fire);
	const int32 Damage = DamageAmount.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float CoolDown = GetCooldown(Level);
	if (Level == 1)
	{
		return FString::Printf(TEXT(""
							  "<Title>FIRE BOLT</>\n\n"
							  
							  "<Small>Level: </><Level>%d</>\n"
							  "<Small>Mana Cost: </><ManaCost>%.1f</>\n" // %.1f will have only one decimal
							  "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
							  
							  "<Default>Launches a bolt of fire, exploding on impact and dealing: </> <Damage>%d</> <Default> fire damage with a chance to burn</>\n\n"
							  ),  Level, ManaCost, CoolDown, Damage);
	}
	return FString::Printf(TEXT(""
		"<Title>FIRE BOLT</>\n\n"
		
		"<Small>Level: </><Level>%d</>\n"
		"<Small>Mana Cost: </><ManaCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		
		"<Default>Launches %d bolts of fire, exploding on impact and dealing: </><Damage>%d</><Default> fire damage with a chance to burn</>\n\n"
		), Level, ManaCost, CoolDown, FMath::Min(Level, NumProjectiles), Damage);
}

FString UFireBolt::GetNextLevelDescription(int32 Level)
{
	const int32 Damage = DamageAmount.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float CoolDown = GetCooldown(Level);
	return FString::Printf(TEXT(""
		"<Title>NEXT LEVEL:</>\n\n"
		
		"<Small>Level: </><Level>%d</>\n"
		"<Small>Mana Cost: </><ManaCost>%.1f</>\n"
		"<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
		
		"<Default>Launches %d bolts of fire, exploding on impact and dealing: </><Damage>%d</><Default> fire damage with a chance to burn</>\n\n"
		), Level, ManaCost, CoolDown, FMath::Min(Level, NumProjectiles), Damage);
}

void UFireBolt::SpawnProjectiles(const FVector& ProjectileTargetLocation, const FGameplayTag& SocketTag,
	const bool bOverridePitch, const float OverridePitch, AActor* HomingTarget)
{
	const bool bIsServer = GetAvatarActorFromActorInfo()->HasAuthority();
	if (!bIsServer) return;

	const FVector SocketLocation = ICombatInterface::Execute_GetCombatSocketLocation(GetAvatarActorFromActorInfo(), SocketTag);

	FRotator Rotation = (ProjectileTargetLocation - SocketLocation).Rotation();
	// Rotation.Pitch = 0.f; // set the projectile parallel to ground

	if (bOverridePitch) Rotation.Pitch = OverridePitch;

	NumProjectiles = FMath::Min(MaxNumProjectiles, GetAbilityLevel());

	const FVector Forward = Rotation.Vector();

	TArray<FRotator> Rotations = USeptentAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, ProjectileSpread, NumProjectiles);
	
	for (FRotator& Rot : Rotations)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(SocketLocation);
		SpawnTransform.SetRotation(Rot.Quaternion());
		
		AProjectile* Projectile = GetWorld()->SpawnActorDeferred<AProjectile>(ProjectileClass, SpawnTransform,
			GetAvatarActorFromActorInfo(), Cast<APawn>(GetOwningActorFromActorInfo()),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn); // Using an Avatar actor as an owner to avoid an effect handle being null on a client

		Projectile->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();

		if (HomingTarget && HomingTarget->Implements<UCombatInterface>())
		{
			Projectile->ProjectileMovement->HomingTargetComponent = HomingTarget->GetRootComponent();
		}
		else
		{
			Projectile->HomingTargetSceneComponent = NewObject<USceneComponent>(USceneComponent::StaticClass());
			Projectile->HomingTargetSceneComponent->SetWorldLocation(ProjectileTargetLocation);
			Projectile->ProjectileMovement->HomingTargetComponent = Projectile->HomingTargetSceneComponent;
		}

		Projectile->ProjectileMovement->HomingAccelerationMagnitude = FMath::FRandRange(HomingAccelerationMin, HomingAccelerationMax);
		Projectile->ProjectileMovement->bIsHomingProjectile = bLaunchHomingProjectiles;
		
		Projectile->FinishSpawning(SpawnTransform);
	}
	
}
