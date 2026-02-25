// ZZ

#pragma once

#include "CoreMinimal.h"
#include "Engine/AssetManager.h"
#include "SeptentAssetManager.generated.h"

/**
 * 
 */
UCLASS()
class SEPTENT_API USeptentAssetManager : public UAssetManager
{
	GENERATED_BODY()

public:
	static USeptentAssetManager& Get();

protected:
	virtual void StartInitialLoading() override;
};
