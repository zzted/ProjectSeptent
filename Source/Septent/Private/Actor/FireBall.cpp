// ZZ


#include "Actor/FireBall.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "SeptentGameplayTags.h"
#include "GameplayCueManager.h"
#include "AbilitySystem/SeptentAbilitySystemLibrary.h"

void AFireBall::ExecuteImpactFXs() 
{
	if (GetOwner())
	{
		FGameplayCueParameters CueParams;
		CueParams.Location = GetActorLocation();
		// Execute gameplay cue locally when something is already replicated
		UGameplayCueManager::ExecuteGameplayCue_NonReplicated(GetOwner(), FSeptentGameplayTags::Get().GameplayCue_FireBlast, CueParams);
	}
	bHit = true;
}

void AFireBall::BeginPlay()
{
	Super::BeginPlay();
	StartOutgoingTimeline();
}

void AFireBall::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                    UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	const AActor* ThisOwner = GetOwner();
	if (!IsValid(ThisOwner))
	{
		bHit = true;
		Destroy();
		return;
	}

	if (OtherActor == ThisOwner) return;
	if (!USeptentAbilitySystemLibrary::IsNotFriend(OtherActor, ThisOwner)) return;

	if (HasAuthority())
	{
		// NOTE: DamageEffectSpecHandle should be valid only on the server (we set it there but also don't replicate it).
		
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			DamageEffectParams.DeathImpulse = DeathImpulse;
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;
			
			USeptentAbilitySystemLibrary::ApplyDamageEffectsFromDamageEffectParams(DamageEffectParams);
		}
	}
}
