// ZZ

#pragma once

#include "CoreMinimal.h"
#include "Character/SeptentCharacterBase.h"
#include "Interaction/PlayerInterface.h"
#include "SeptentCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
/**
 * @class ASeptentCharacter
 * @brief ASeptentCharacter is a specific character class derived from ASeptentCharactorBase.
 * It adds functionality for initializing ability system information and handling player state replication.
 */
UCLASS()
class SEPTENT_API ASeptentCharacter : public ASeptentCharacterBase, public IPlayerInterface
{
	GENERATED_BODY()

public:
	ASeptentCharacter();

	virtual void PossessedBy(AController* NewController) override;

	virtual void OnRep_PlayerState() override;

	/* Player Interface*/
	virtual void AddToXP_Implementation(int32 InXP) override;
	virtual void LevelUp_Implementation() override;
	virtual int32 GetXP_Implementation() const override;
	virtual int32 FindLevelForXP_Implementation(int32 InXP) const override;
	virtual int32 GetAttributePointsReward_Implementation(int32 InPlayerLevel) const override;
	virtual int32 GetSpellPointsReward_Implementation(int32 InPlayerLevel) const override;
	virtual void AddToPlayerLevel_Implementation(int32 InPlayerLevel) override;
	virtual void AddToAttributePoints_Implementation(int32 InAttributePoints) override;
	virtual void AddToSpellPoints_Implementation(int32 InSpellPoints) override;
	virtual int32 GetAttributePoints_Implementation() const override;
	virtual int32 GetSpellPoints_Implementation() const override;
	virtual void ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial = nullptr) override;
	virtual void HideMagicCircle_Implementation() override;
	virtual void SaveProgress_Implementation(const FName& CheckpointTag) override;
	/* end Player Interface*/

	/* Combat Interface*/
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual void Die(const FVector& DeathImpulse) override;
	/* end Combat Interface*/

	UPROPERTY(EditDefaultsOnly)
	float DeathTime = 5.f;

	FTimerHandle DeathTimerHandle;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UNiagaraComponent> LevelUpNiagaraComponent;

	virtual void OnRep_Stunned() override;
	virtual void OnRep_Burned() override;

	FTimerHandle InitASCRetryTimerHandle;
	int32 InitASCRetryCount = 0;
	static constexpr int32 MaxInitASCRetryCount = 5;

	void RetryInitAbilityActorInfo();

	void LoadProgress() const;

private:

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UCameraComponent> CameraComponent;

	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USpringArmComponent> SpringArmComponent;
	
	virtual void InitAbilityActorInfo() override;

	UFUNCTION(NetMulticast, Reliable)
	void MulticastLevelUpParticles() const;
};
