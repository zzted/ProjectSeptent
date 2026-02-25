// ZZ

#pragma once

#include "CoreMinimal.h"
#include "Actor/Projectile.h"
#include "FireBall.generated.h"

/**
 * 
 */
UCLASS()
class SEPTENT_API AFireBall : public AProjectile
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void StartOutgoingTimeline();

	virtual void ExecuteImpactFXs() override;

	UPROPERTY(BlueprintReadOnly)
	TObjectPtr<AActor> ReturnToActor;

	UPROPERTY(BlueprintReadWrite)
	FDamageEffectParams ExplosionDamageEffectParams;

protected:
	virtual void BeginPlay() override;
	virtual void OnSphereOverlap(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult) override;
};
