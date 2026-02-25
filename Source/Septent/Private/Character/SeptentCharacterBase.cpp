// ZZ


#include "Character/SeptentCharacterBase.h"

#include "AbilitySystemComponent.h"
#include "SeptentGameplayTags.h"
#include "MotionWarpingComponent.h"
#include "AbilitySystem/SeptentAbilitySystemComponent.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "AbilitySystem/Passive/PassiveNiagaraComponent.h"
#include "Septent/Septent.h"
#include "Components/CapsuleComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Net/UnrealNetwork.h"

class UMotionWarpingComponent;

ASeptentCharacterBase::ASeptentCharacterBase()
{
	PrimaryActorTick.bCanEverTick = false;

	BurnDebuffNiagaraComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>("BurnDebuffNiagaraComponent");
	BurnDebuffNiagaraComponent->SetupAttachment(GetRootComponent());
	BurnDebuffNiagaraComponent->DebuffTag = FSeptentGameplayTags::Get().Debuff_Burn;

	StunDebuffNiagaraComponent = CreateDefaultSubobject<UDebuffNiagaraComponent>("StunDebuffNiagaraComponent");
	StunDebuffNiagaraComponent->SetupAttachment(GetRootComponent());
	StunDebuffNiagaraComponent->DebuffTag = FSeptentGameplayTags::Get().Debuff_Stun;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetCapsuleComponent()->SetGenerateOverlapEvents(false);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Projectile, ECR_Overlap);
	GetMesh()->SetGenerateOverlapEvents(true);
	
	Weapon = CreateDefaultSubobject<USkeletalMeshComponent>("Weapon");
	Weapon->SetupAttachment(GetMesh(), FName("WeaponHandSocket"));
	Weapon->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	EffectAttachComponent = CreateDefaultSubobject<USceneComponent>("EffectAttachPoint");
	EffectAttachComponent->SetupAttachment(GetRootComponent());
	HaloOfProtectionNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>("HaloOfProtectionNiagaraComponent");
	HaloOfProtectionNiagaraComponent->SetupAttachment(EffectAttachComponent);
	LifeSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>("LifeSiphonNiagaraComponent");
	LifeSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);
	ManaSiphonNiagaraComponent = CreateDefaultSubobject<UPassiveNiagaraComponent>("ManaSiphonNiagaraComponent");
	ManaSiphonNiagaraComponent->SetupAttachment(EffectAttachComponent);
}

void ASeptentCharacterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	EffectAttachComponent->SetWorldRotation(FRotator::ZeroRotator);
}

void ASeptentCharacterBase::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASeptentCharacterBase, bIsStunned);
	DOREPLIFETIME(ASeptentCharacterBase, bIsBurned);
	DOREPLIFETIME(ASeptentCharacterBase, bIsBeingShocked);
}

float ASeptentCharacterBase::TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent,
	class AController* EventInstigator, AActor* DamageCauser)
{
	const float DamageTaken = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);
	OnDamageDelegate.Broadcast(DamageTaken);
	return DamageTaken;
}

UAbilitySystemComponent* ASeptentCharacterBase::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

UAnimMontage* ASeptentCharacterBase::GetHitReactMontage_Implementation()
{
	return HitReactMontage;
}

void ASeptentCharacterBase::Die(const FVector& DeathImpulse)
{
	Weapon->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, true));
	MulticastHandleDeath(DeathImpulse);
}

void ASeptentCharacterBase::MulticastHandleDeath_Implementation(const FVector& DeathImpulse)
{
	UGameplayStatics::PlaySoundAtLocation(this, DeathSound, GetActorLocation(), GetActorRotation());
	Weapon->SetSimulatePhysics(true);
	Weapon->SetEnableGravity(true);
	Weapon->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	Weapon->AddImpulse(DeathImpulse * .1f, NAME_None, true);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetEnableGravity(true);
	GetMesh()->SetCollisionEnabled(ECollisionEnabled::PhysicsOnly);
	GetMesh()->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);
	GetMesh()->AddImpulse(DeathImpulse, NAME_None, true);
	
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	Dissolve();
	bIsDead = true;
	OnDeathDelegate.Broadcast(this);
}

void ASeptentCharacterBase::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bIsStunned = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bIsStunned ? 0.f : BaseWalkSpeed;
	
}

void ASeptentCharacterBase::BeginPlay()
{
	Super::BeginPlay();
}

void ASeptentCharacterBase::InitAbilityActorInfo()
{
}

FVector ASeptentCharacterBase::GetCombatSocketLocation_Implementation(const FGameplayTag& MontageTag)
{
	// TODO Return correct socket based on MontageTag
	const FSeptentGameplayTags& GameplayTags = FSeptentGameplayTags::Get();
	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Weapon) && IsValid(Weapon))
	{
		return Weapon->GetSocketLocation(WeaponTipSocketName);
	}

	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_LeftHand))
	{
		return GetMesh()->GetSocketLocation(LeftHandSocketName);
	}

	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_RightHand))
	{
		return GetMesh()->GetSocketLocation(RightHandSocketName);
	}

	if (MontageTag.MatchesTagExact(GameplayTags.CombatSocket_Tail))
	{
		return GetMesh()->GetSocketLocation(TailSocketName);
	}
	return FVector::ZeroVector;
}

bool ASeptentCharacterBase::IsDead_Implementation() const
{
	return bIsDead;
}

AActor* ASeptentCharacterBase::GetAvatar_Implementation()
{
	return this;
}

TArray<FTaggedMontage> ASeptentCharacterBase::GetAttackMontages_Implementation()
{
	return AttackMontages;
}

UNiagaraSystem* ASeptentCharacterBase::GetBloodEffect_Implementation()
{
	return BloodEffect;
}

FTaggedMontage ASeptentCharacterBase::GetTaggedMontageByTag_Implementation(const FGameplayTag& MontageTag)
{
	for (FTaggedMontage TaggedMontage : AttackMontages)
	{
		if (TaggedMontage.MontageTag.MatchesTagExact(MontageTag))
		{
			return TaggedMontage;
		}
	}
	return FTaggedMontage();
}

int32 ASeptentCharacterBase::GetMinionCount_Implementation()
{
	return MinionCount;
}

void ASeptentCharacterBase::IncrementMinionCount_Implementation(const int32 Amount)
{
	MinionCount += Amount;
}

ECharacterClass ASeptentCharacterBase::GetCharacterClass_Implementation()
{
	return CharacterClass;
}

FOnASCRegistered& ASeptentCharacterBase::GetOnASCRegisteredDelegate()
{
	return OnAscRegistered;
}

FOnDeathSignature& ASeptentCharacterBase::GetOnDeathDelegate()
{
	return OnDeathDelegate;
}

FOnDamageSignature& ASeptentCharacterBase::GetOnDamageSignature()
{
	return OnDamageDelegate;
}

USkeletalMeshComponent* ASeptentCharacterBase::GetWeapon_Implementation() const
{
	return Weapon;
}

bool ASeptentCharacterBase::IsBeingShocked_Implementation() const
{
	return bIsBeingShocked;
}

void ASeptentCharacterBase::SetBeingShocked_Implementation(const bool bInShock)
{
	bIsBeingShocked = bInShock;
}

void ASeptentCharacterBase::OnRep_Stunned()
{
}

void ASeptentCharacterBase::OnRep_Burned()
{
}

void ASeptentCharacterBase::ApplyEffectToSelf(const TSubclassOf<UGameplayEffect>& EffectToApply, const float Level) const
{
	check(IsValid(GetAbilitySystemComponent()));
	check(EffectToApply);
	FGameplayEffectContextHandle EffectContextHandle = GetAbilitySystemComponent()->MakeEffectContext();
	EffectContextHandle.AddSourceObject(this);
	const FGameplayEffectSpecHandle EffectSpecHandle = GetAbilitySystemComponent()->MakeOutgoingSpec(EffectToApply, Level, EffectContextHandle);
	GetAbilitySystemComponent()->ApplyGameplayEffectSpecToTarget(*EffectSpecHandle.Data.Get(), GetAbilitySystemComponent());
}

void ASeptentCharacterBase::InitializeDefaultAttributes() const
{
	ApplyEffectToSelf(DefaultPrimaryAttributes, 1.f);
	ApplyEffectToSelf(DefaultSecondaryAttributes, 1.f);
	ApplyEffectToSelf(InitialVitalAttributes, 1.f);
}

void ASeptentCharacterBase::AddCharacterAbilities() const
{
	USeptentAbilitySystemComponent* SeptentAbilitySystemComponent = Cast<USeptentAbilitySystemComponent>(AbilitySystemComponent);
	if (!HasAuthority()) return;

	SeptentAbilitySystemComponent->AddCharacterAbilities(StartupAbilities);
	SeptentAbilitySystemComponent->AddCharacterPassiveAbilities(StartupPassiveAbilities);
}

void ASeptentCharacterBase::Dissolve()
{
	if (IsValid(DissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(DissolveMaterialInstance, this);
		GetMesh()->SetMaterial(0, DynamicMaterialInstance);
		StartDissolveTimeline(DynamicMaterialInstance);
	}

	if (IsValid(WeaponDissolveMaterialInstance))
	{
		UMaterialInstanceDynamic* DynamicMaterialInstance = UMaterialInstanceDynamic::Create(WeaponDissolveMaterialInstance, this);
		Weapon->SetMaterial(0, DynamicMaterialInstance);
		StartWeaponDissolveTimeline(DynamicMaterialInstance);
	}
}
