// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Inv_ProxyMeshActor.generated.h"

class UInv_EquipmentComponent;

UCLASS()
class INVENTORY_API AInv_ProxyMeshActor : public AActor
{
	GENERATED_BODY()

public:
	AInv_ProxyMeshActor();
	
	USkeletalMeshComponent* GetProxyMesh() const { return ProxyMesh; }

protected:
	virtual void BeginPlay() override;
	
private:
	// The mesh on player controlled character
	TWeakObjectPtr<USkeletalMeshComponent> SourceMesh;
	
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<UInv_EquipmentComponent> EquipmentComponent;
	
	// The proxy mesh we see in the inventory menu
	UPROPERTY(VisibleAnywhere)
	TObjectPtr<USkeletalMeshComponent> ProxyMesh;
	
	FTimerHandle TimerForNextTick;
	void DelayedInitializeOwner();
	void DelayedInitialization();
};
