// ZZ

#pragma once

#include "CoreMinimal.h"
#include "SeptentAssetManager.h"
#include "Engine/GameInstance.h"
#include "SeptentGameInstance.generated.h"

/**
 * 
 */
UCLASS()
class SEPTENT_API USeptentGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FName PlayerStartTag = FName();

	UPROPERTY()
	FString LoadSlotName = FString();

	UPROPERTY()
	int32 LoadSlotIndex = 0;
};
