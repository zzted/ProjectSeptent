// ZZ


#include "SeptentAssetManager.h"
#include "SeptentGameplayTags.h"
#include "AbilitySystemGlobals.h"

USeptentAssetManager& USeptentAssetManager::Get()
{
	check(GEngine);
	
	USeptentAssetManager* SeptentAssetManager = Cast<USeptentAssetManager>(GEngine->AssetManager);
	return *SeptentAssetManager;
}

void USeptentAssetManager::StartInitialLoading()
{
	Super::StartInitialLoading();
	
	// FSeptentGameplayTags::InitializeNativeGameplayTags(); Was called here but is causing a problem that the tags are not initialized when DamageStatics is constructed. changed to a singleton type initialization.

	//UAbilitySystemGlobals::Get().InitGlobalData();  TargetDataStructCache init, required for using target data. no need to call that after UE 5.3 https://github.com/tranek/GASDocumentation?tab=readme-ov-file#491-initglobaldata
}
