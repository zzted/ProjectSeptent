// ZZ

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameFramework/PlayerState.h"
#include "SeptentPlayerState.generated.h"

class ULevelUpInfo;
class UAttributeSet;
class UAbilitySystemComponent;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPlayerStatChangedSignature, int32);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPlayerLevelChangedSignature, int32 /*LevelValue*/, bool /*bLevelUp*/)
/**
 * 
 */
UCLASS()
class SEPTENT_API ASeptentPlayerState : public APlayerState, public IAbilitySystemInterface
{
	GENERATED_BODY()

public:
	ASeptentPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	virtual UAbilitySystemComponent* GetAbilitySystemComponent() const override;

	UAttributeSet* GetAttributeSet() const { return AttributeSet; };

	UPROPERTY(EditDefaultsOnly)
	TObjectPtr<ULevelUpInfo> LevelUpInfo;

	FORCEINLINE int32 GetPlayerLevel() const { return Level; }
	FORCEINLINE int32 GetXP() const { return XP; }
	FORCEINLINE int32 GetAttributePoints() const { return AttributePoints; }
	FORCEINLINE int32 GetSpellPoints() const { return SpellPoints; }

	FOnPlayerStatChangedSignature OnXPChangedDelegate;
	FOnPlayerLevelChangedSignature OnLevelChangedDelegate;
	FOnPlayerStatChangedSignature OnAttributePointsChangedDelegate;
	FOnPlayerStatChangedSignature OnSpellPointsChangedDelegate;

	void SetXP(const int32 InXP);
	void SetLevel(const int32 InLevel);
	void SetAttributePoints(const int32 InAttributePoints);
	void SetSpellPoints(const int32 InSpellPoints);
	
	void AddToXP(const int32 InXP);
	void AddToLevel(const int32 InLevel);
	void AddToAttributePoints(const int32 InAttributePoints);
	void AddToSpellPoints(const int32 InSpellPoints);

protected:
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UAbilitySystemComponent> AbilitySystemComponent;

	UPROPERTY()
	TObjectPtr<UAttributeSet> AttributeSet;

private:
	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_Level)
	int32 Level = 1;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_XP)
	int32 XP = 0;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_AttributePoints)
	int32 AttributePoints = 0;

	UPROPERTY(VisibleAnywhere, ReplicatedUsing = OnRep_SpellPoints)
	int32 SpellPoints = 0;

	UFUNCTION()
	void OnRep_Level() const;

	UFUNCTION()
	void OnRep_XP() const;

	UFUNCTION()
	void OnRep_AttributePoints() const;

	UFUNCTION()
	void OnRep_SpellPoints() const;
};
