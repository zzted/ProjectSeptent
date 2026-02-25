// ZZ


#include "Player/SeptentPlayerState.h"

#include "AbilitySystem/SeptentAbilitySystemComponent.h"
#include "AbilitySystem/SeptentAttributeSet.h"
#include "Net/UnrealNetwork.h"

ASeptentPlayerState::ASeptentPlayerState()
{

	AbilitySystemComponent = CreateDefaultSubobject<USeptentAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Mixed);

	AttributeSet = CreateDefaultSubobject<USeptentAttributeSet>("AttributeSet");
	NetUpdateFrequency = 100.f;
}

void ASeptentPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(ASeptentPlayerState, Level); // register variable to replication, important
	DOREPLIFETIME(ASeptentPlayerState, XP);
	DOREPLIFETIME(ASeptentPlayerState, AttributePoints);
	DOREPLIFETIME(ASeptentPlayerState, SpellPoints);
}

UAbilitySystemComponent* ASeptentPlayerState::GetAbilitySystemComponent() const
{
	return AbilitySystemComponent;
}

void ASeptentPlayerState::SetXP(const int32 InXP)
{
	XP = InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void ASeptentPlayerState::SetLevel(const int32 InLevel)
{
	Level = InLevel;
	OnLevelChangedDelegate.Broadcast(Level, false);
}


void ASeptentPlayerState::SetAttributePoints(const int32 InAttributePoints)
{
	AttributePoints = InAttributePoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void ASeptentPlayerState::SetSpellPoints(const int32 InSpellPoints)
{
	SpellPoints = InSpellPoints;
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

void ASeptentPlayerState::AddToXP(const int32 InXP)
{
	XP += InXP;
	OnXPChangedDelegate.Broadcast(XP);
}

void ASeptentPlayerState::AddToLevel(const int32 InLevel)
{
	Level += InLevel;
	OnLevelChangedDelegate.Broadcast(Level, true);
}

void ASeptentPlayerState::AddToAttributePoints(const int32 InAttributePoints)
{
	AttributePoints += InAttributePoints;
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void ASeptentPlayerState::AddToSpellPoints(const int32 InSpellPoints)
{
	SpellPoints += InSpellPoints;
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}

void ASeptentPlayerState::OnRep_Level() const
{
	OnLevelChangedDelegate.Broadcast(Level, true);
}

void ASeptentPlayerState::OnRep_XP() const
{
	OnXPChangedDelegate.Broadcast(XP);
}

void ASeptentPlayerState::OnRep_AttributePoints() const
{
	OnAttributePointsChangedDelegate.Broadcast(AttributePoints);
}

void ASeptentPlayerState::OnRep_SpellPoints() const
{
	OnSpellPointsChangedDelegate.Broadcast(SpellPoints);
}
