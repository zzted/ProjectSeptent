// ZZ

#pragma once

#include "CoreMinimal.h"
#include "Character/SeptentCharacterBase.h"
#include "Interaction/EnemyInterface.h"
#include "Interaction/HighlightInterface.h"
#include "UI/WidgetController/OverlayWidgetController.h"
#include "SeptentEnemy.generated.h"

class UWidgetComponent;
class UBehaviorTree;
class ASeptentAIController;
/**
 * @class ASeptentEnemy
 * @brief Represents an enemy character in the Septent game, inheriting from ASeptentCharactorBase and implementing the IEnemyInterface.
 *
 * This class is responsible for defining behavior specific to enemy characters, including highlighting and un-highlighting
 * functionality as part of the implemented IEnemyInterface. It also initializes its own Ability System Component and Attribute Set
 * for gameplay-related features.
 */
UCLASS()
class SEPTENT_API ASeptentEnemy : public ASeptentCharacterBase, public IEnemyInterface, public IHighlightInterface
{
	GENERATED_BODY()
	
public:
	ASeptentEnemy();

	virtual void PossessedBy(AController* NewController) override;

	/* Highlight Interface */
	virtual void HighlightActor_Implementation() override;
	virtual void UnHighlightActor_Implementation() override;
	virtual void SetMoveToLocation_Implementation(FVector& OutDestination) override;
	/* end Highlight Interface */

	/* Combat Interface */
	virtual int32 GetPlayerLevel_Implementation() override;
	virtual void Die(const FVector& DeathImpulse) override;
	virtual void SetCombatTarget_Implementation(AActor* InCombatTarget) override;
	virtual AActor* GetCombatTarget_Implementation() const override;
	/* end Combat Interface */

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnHealthChanged;

	UPROPERTY(BlueprintAssignable)
	FOnAttributeChangedSignature OnMaxHealthChanged;

	void HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount);

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	bool bHitReacting = false;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Combat")
	float LifeSpan = 5.f;

	UPROPERTY(BlueprintReadOnly, Category = "Combat")
	TObjectPtr<AActor> CombatTarget;

	void SetLevel(int32 InLevel) { Level = InLevel; }

protected:
	virtual void BeginPlay() override;
	virtual void InitAbilityActorInfo() override;
	virtual void InitializeDefaultAttributes() const override;
	virtual void StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount) override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character Class Defaults")
	int32 Level = 1;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	TObjectPtr<UWidgetComponent> HealthBar;

	UPROPERTY(EditAnywhere, Category = "AI")
	TObjectPtr<UBehaviorTree> BehaviorTree;

	UPROPERTY()
	TObjectPtr<ASeptentAIController> SeptentAIController;

	UFUNCTION(BlueprintImplementableEvent)
	void SpawnLoot();
};
