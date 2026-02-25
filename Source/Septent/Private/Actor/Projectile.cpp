// ZZ


#include "Actor/Projectile.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/SeptentAbilitySystemLibrary.h"
#include "Septent/Septent.h"
#include "Components/SphereComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"


AProjectile::AProjectile()
{
	PrimaryActorTick.bCanEverTick = false;
	bReplicates = true;
	
	
	Sphere = CreateDefaultSubobject<USphereComponent>("Sphere");
	SetRootComponent(Sphere);
	Sphere->SetCollisionObjectType(ECC_Projectile);
	Sphere->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	Sphere->SetCollisionResponseToAllChannels(ECR_Ignore);
	Sphere->SetCollisionResponseToChannel(ECC_WorldDynamic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Overlap);
	Sphere->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>("ProjectileMovement");
	ProjectileMovement->InitialSpeed = 550.f;
	ProjectileMovement->MaxSpeed = 550.f;
	ProjectileMovement->ProjectileGravityScale = 0.f;
}

void AProjectile::BeginPlay()
{
	Super::BeginPlay();
	SetLifeSpan(LifeSpan);
	SetReplicateMovement(true);
	Sphere->OnComponentBeginOverlap.AddDynamic(this, &AProjectile::OnSphereOverlap);
	LoopingSoundComponent = UGameplayStatics::SpawnSoundAttached(LoopingSound, GetRootComponent(), NAME_None,
		FVector::Zero(), FRotator::ZeroRotator,
		EAttachLocation::KeepRelativeOffset,
		true); // Don't need to stop looping sound when we marked this as true
}

void AProjectile::Destroyed()
{

	// Execute the FXs on the client only in the case this projectile's destruction was replicated
	// before OnBeginOverlap() had a chance to execute.
	if (!bHit && !HasAuthority())
	{
		// const FString NetPrefix = GetWorld()->IsNetMode(NM_Client) ? "[CLIENT] " : "[SERVER] ";
		// GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Red, NetPrefix + TEXT("Play Sound -- Destroyed"));
 
		ExecuteImpactFXs();
	}
	
	Super::Destroyed();
}

void AProjectile::OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor,
                                      UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	// NOTE: We set this projectile's Owner (as well as the Instigator, both replicated properties from AActor)
	// equal to the source ASC's Avatar Actor (character) through UWorld::SpawnActorDeferred().
 
	AActor* ThisOwner = GetOwner();

	// If the Owner is invalid, destroy this projectile without causing damage or executing the VFX/SFX.
	// This handles the case when Destroy() has already being called on the Owner (we currently set
	// a brief lifespan on death for enemy characters), so it's invalid (i.e., either pending kill or null).
	// If we apply a damage GE while the source character (Owner) is invalid, it ends up triggering an exception in
	// UExecCalc_Damage::Execute_Implementation() caused by ExecutionParams.GetSourceAbilitySystemComponent() being null.
	// FIXME: Find a way to allow a projectile with an already destroyed Owner to do damage.
	// Potential solution: From the base character keep track of the projectiles' lifetime, and when the character dies,
	// delay calling Destroy() until all projectiles have been destroyed.

	if (!IsValid(ThisOwner))
	{
		bHit = true;
		Destroy();
		return;
	}

	if (OtherActor == ThisOwner) return;
	if (!USeptentAbilitySystemLibrary::IsNotFriend(OtherActor, ThisOwner)) return;
	// const FString NetPrefix = GetWorld()->IsNetMode(NM_Client) ? "[CLIENT] " : "[SERVER] ";
	// GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Magenta, NetPrefix + TEXT("Play Sound -- OnBeginOverlap"));

	ExecuteImpactFXs();

	if (HasAuthority())
	{
		// NOTE: DamageEffectSpecHandle should be valid only on the server (we set it there but also don't replicate it).
		
		if (UAbilitySystemComponent* TargetASC = UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(OtherActor))
		{
			const FVector DeathImpulse = GetActorForwardVector() * DamageEffectParams.DeathImpulseMagnitude;
			DamageEffectParams.DeathImpulse = DeathImpulse;
			DamageEffectParams.TargetAbilitySystemComponent = TargetASC;

			const bool bIsKnockback = FMath::RandRange(0.f, 100.f) < DamageEffectParams.KnockbackChance;
			if (bIsKnockback)
			{
				FRotator Rotation =  GetActorRotation();
				Rotation.Pitch = 45.f;
				
				// const FVector KnockbackDirection = GetActorForwardVector().RotateAngleAxis(45.f, GetActorRightVector());
				const FVector KnockbackDirection = Rotation.Vector();
				const FVector KnockbackForce = KnockbackDirection * DamageEffectParams.KnockbackForceMagnitude;
				DamageEffectParams.KnockbackForce = KnockbackForce;
			}
			
			USeptentAbilitySystemLibrary::ApplyDamageEffectsFromDamageEffectParams(DamageEffectParams);
		}
		Destroy();
	}
	else
	{
		// Hide the actor and disable collision.
		// This handles a potential delay in the replication of this projectile's destruction.
		// It also prevents OnBeginOverlap() to execute more than once for the client.
		SetActorHiddenInGame(true);
		Sphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
		bHit = true;
	}
}

void AProjectile::ExecuteImpactFXs()
{
	UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation(), FRotator::ZeroRotator);
	UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ImpactEffect, GetActorLocation());
}
