// ZZ


#include "AbilitySystem/Abilities/FireBlast.h"

#include "AbilitySystem/SeptentAbilitySystemLibrary.h"
#include "Actor/FireBall.h"

FString UFireBlast::GetDescription(int32 Level)
{
	const int32 Damage = DamageAmount.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float CoolDown = GetCooldown(Level);

	return FString::Printf(TEXT(""
						  "<Title>FIRE BLAST</>\n\n"
						  
						  "<Small>Level: </><Level>%d</>\n"
						  "<Small>Mana Cost: </><ManaCost>%.1f</>\n" // %.1f will have only one decimal
						  "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
						  
						  "<Default>Launches %d fire balls in all direction, "
						  "each coming back and exploding upon return, "
						  "causing: </> <Damage>%d</> <Default> radial fire damage with a chance to burn</>\n\n"
						  ),  Level, ManaCost, CoolDown, NumFireBalls, Damage);
}

FString UFireBlast::GetNextLevelDescription(int32 Level)
{
	const int32 Damage = DamageAmount.GetValueAtLevel(Level);
	const float ManaCost = FMath::Abs(GetManaCost(Level));
	const float CoolDown = GetCooldown(Level);

	return FString::Printf(TEXT(""
						  "<Title>NEXT LEVEL:</>\n\n"
						  
						  "<Small>Level: </><Level>%d</>\n"
						  "<Small>Mana Cost: </><ManaCost>%.1f</>\n" // %.1f will have only one decimal
						  "<Small>Cooldown: </><Cooldown>%.1f</>\n\n"
						  
						  "<Default>Launches %d fire balls in all direction, "
						  "each coming back and exploding upon return, "
						  "causing: </> <Damage>%d</> <Default> radial fire damage with a chance to burn</>\n\n"
						  ),  Level, ManaCost, CoolDown, NumFireBalls, Damage);
}

TArray<AFireBall*> UFireBlast::SpawnFireBalls()
{
	TArray<AFireBall*> FireBalls;
	const FVector Forward = GetAvatarActorFromActorInfo()->GetActorForwardVector();
	const FVector Location = GetAvatarActorFromActorInfo()->GetActorLocation();
	TArray<FRotator> Rotators = USeptentAbilitySystemLibrary::EvenlySpacedRotators(Forward, FVector::UpVector, 360.f, NumFireBalls);

	for (const FRotator& Rotator : Rotators)
	{
		FTransform SpawnTransform;
		SpawnTransform.SetLocation(Location);
		SpawnTransform.SetRotation(Rotator.Quaternion());
		AFireBall* FireBall = GetWorld()->SpawnActorDeferred<AFireBall>(
			FireBallClass,
			SpawnTransform,
			GetAvatarActorFromActorInfo(),
			CurrentActorInfo->PlayerController->GetPawn(),
			ESpawnActorCollisionHandlingMethod::AlwaysSpawn);

		FireBall->DamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->ReturnToActor = GetAvatarActorFromActorInfo();

		FireBall->ExplosionDamageEffectParams = MakeDamageEffectParamsFromClassDefaults();
		FireBall->SetOwner(GetAvatarActorFromActorInfo());

		FireBalls.Add(FireBall);

		FireBall->FinishSpawning(SpawnTransform);
	}
	
	return FireBalls;
}
