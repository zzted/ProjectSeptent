// ZZ

#pragma once

#include "CoreMinimal.h"
#include "AIController.h"
#include "SeptentAIController.generated.h"

class UBlackboardComponent;
class UBehaviorTreeComponent;

/**
 * 
 */
UCLASS()
class SEPTENT_API ASeptentAIController : public AAIController
{
	GENERATED_BODY()

	
public:
	ASeptentAIController();

protected:

	UPROPERTY()
	TObjectPtr<UBehaviorTreeComponent> BehaviorTreeComponent;
};
