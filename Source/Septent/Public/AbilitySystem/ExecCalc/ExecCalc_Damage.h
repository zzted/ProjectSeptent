// ZZ

#pragma once

#include "CoreMinimal.h"
#include "GameplayEffectExecutionCalculation.h"
#include "ExecCalc_Damage.generated.h"

/**
 * 
 */
UCLASS()
class SEPTENT_API UExecCalc_Damage : public UGameplayEffectExecutionCalculation
{
	GENERATED_BODY()

public:
	UExecCalc_Damage();
	void DetermineDebuffs(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
	                      const FGameplayEffectSpec& Spec,
	                      const FAggregatorEvaluateParameters& EvaluationParameters) const;

	virtual void Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams, FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const override;
};
