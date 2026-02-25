// ZZ

#pragma once

#include "CoreMinimal.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Engine/TargetPoint.h"
#include "EnemySpawnPoint.generated.h"

class ASeptentEnemy;
/**
 * 
 */
UCLASS()
class SEPTENT_API AEnemySpawnPoint : public ATargetPoint
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable)
	void SpawnEnemy();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy Class")
	TSubclassOf<ASeptentEnemy> EnemyClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy Class")
	int32 EnemyLevel = 1;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Enemy Class")
	ECharacterClass CharacterClass = ECharacterClass::Warrior;
};
