// ZZ


#include "AbilitySystem/Abilities/BeamSpell.h"

#include "AbilitySystem/SeptentAbilitySystemLibrary.h"
#include "GameFramework/Character.h"
#include "Kismet/KismetSystemLibrary.h"

void UBeamSpell::StoreMouseDataInfo(const FHitResult& HitResult)
{
	if (HitResult.IsValidBlockingHit())
	{
		MouseHitLocation = HitResult.Location;
		MouseHitActor = HitResult.GetActor();
	} else
	{
		CancelAbility(CurrentSpecHandle, CurrentActorInfo, CurrentActivationInfo, true);
	}
}

void UBeamSpell::StoreOwnerVariables()
{
	if (CurrentActorInfo)
	{
		OwnerPlayerController = CurrentActorInfo->PlayerController.Get();
		OwnerCharacter = Cast<ACharacter>(CurrentActorInfo->AvatarActor);
	}
}

void UBeamSpell::TraceFirstTarget(const FVector& BeamTargetLocation)
{
	check(OwnerCharacter);
	if (OwnerCharacter->Implements<UCombatInterface>())
	{
		if (USkeletalMeshComponent* Weapon = ICombatInterface::Execute_GetWeapon(OwnerCharacter))
		{
			TArray<AActor*> IgnoreActors;
			IgnoreActors.Add(OwnerCharacter);
			FHitResult HitResult;
			const FVector SocketLocation = Weapon->GetSocketLocation(FName("TipSocket"));
			// Using Sphere trace to save performance 
			UKismetSystemLibrary::SphereTraceSingle(
				OwnerCharacter, SocketLocation,
				BeamTargetLocation, 10.f,
				TraceTypeQuery1, false,
				IgnoreActors, EDrawDebugTrace::None,
				HitResult, true);

			if (HitResult.IsValidBlockingHit())
			{
				MouseHitLocation = HitResult.ImpactPoint;
				MouseHitActor = HitResult.GetActor();
			}
		}
	}
	
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(MouseHitActor))
	{
		if (!CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &UBeamSpell::PrimaryTargetDied))
		{
			CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UBeamSpell::PrimaryTargetDied);
		}
	}
}

void UBeamSpell::StoreAdditionalTargets(TArray<AActor*>& OutAdditionalTargets)
{
	TArray<AActor*> OverlappingActors;
	TArray<AActor*> IgnoringActors;
	IgnoringActors.Add(OwnerCharacter);
	IgnoringActors.Add(MouseHitActor);
	USeptentAbilitySystemLibrary::GetLivePlayersWithinRadius(
		GetAvatarActorFromActorInfo(),OverlappingActors,
		IgnoringActors, 850.f, MouseHitActor->GetActorLocation());

	const int32 NumAdditionalTargets = FMath::Min(MaxNumShockTargets, GetAbilityLevel() - 1);

	USeptentAbilitySystemLibrary::GetClosestTargets(NumAdditionalTargets, OverlappingActors, OutAdditionalTargets, MouseHitActor->GetActorLocation());

	for (AActor* Target : OutAdditionalTargets)
	{
		if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(Target))
		{
			if (!CombatInterface->GetOnDeathDelegate().IsAlreadyBound(this, &UBeamSpell::AdditionalTargetDied))
			{
				CombatInterface->GetOnDeathDelegate().AddDynamic(this, &UBeamSpell::AdditionalTargetDied);
			}
		}
	}
}

void UBeamSpell::RemoveTargetFromOnDeathDelegate(AActor* TargetActor)
{
	if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetActor))
	{
		CombatInterface->GetOnDeathDelegate().RemoveDynamic(this, &UBeamSpell::PrimaryTargetDied);
		CombatInterface->GetOnDeathDelegate().RemoveDynamic(this, &UBeamSpell::AdditionalTargetDied);
	}
}

