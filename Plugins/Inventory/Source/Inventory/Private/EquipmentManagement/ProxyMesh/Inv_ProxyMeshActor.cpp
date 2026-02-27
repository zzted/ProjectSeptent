// Fill out your copyright notice in the Description page of Project Settings.


#include "EquipmentManagement/ProxyMesh/Inv_ProxyMeshActor.h"

#include "EquipmentManagement/Components/Inv_EquipmentComponent.h"
#include "GameFramework/Character.h"


AInv_ProxyMeshActor::AInv_ProxyMeshActor()
{
	PrimaryActorTick.bCanEverTick = false;
	
	// Need also to uncheck run under one process in editor preference to run properly 
	SetReplicates(false);
	
	RootComponent = CreateDefaultSubobject<USceneComponent>("Root");
	
	ProxyMesh = CreateDefaultSubobject<USkeletalMeshComponent>("ProxyMesh");
	ProxyMesh->SetupAttachment(RootComponent);
	
	EquipmentComponent = CreateDefaultSubobject<UInv_EquipmentComponent>("Equipment");
	EquipmentComponent->SetOwningSkeletalMesh(ProxyMesh);
	EquipmentComponent->SetIsProxy(true);
}

void AInv_ProxyMeshActor::BeginPlay()
{
	Super::BeginPlay();
	// Due to network initialization order, required references may not exist at BeginPlay, so we retry next tick.
	DelayedInitializeOwner();
}

void AInv_ProxyMeshActor::DelayedInitializeOwner()
{
	UWorld* World = GetWorld();
	if (!IsValid(World))
	{
		DelayedInitialization();
		return;
	}
	
	APlayerController* PC = World->GetFirstPlayerController();
	if (!IsValid(PC))
	{
		DelayedInitialization();
		return;
	}
	
	ACharacter* Character = Cast<ACharacter>(PC->GetPawn());
	if (!IsValid(Character))
	{
		DelayedInitialization();
		return;
	}
	
	USkeletalMeshComponent* CharacterMesh = Character->GetMesh();
	if (!IsValid(CharacterMesh))
	{
		DelayedInitialization();
	}
	SourceMesh = CharacterMesh;
	ProxyMesh->SetSkeletalMesh(SourceMesh->GetSkeletalMeshAsset());
	// Set proxy mesh to have the same ABP
	ProxyMesh->SetAnimInstanceClass(SourceMesh->GetAnimInstance()->GetClass());
	
	EquipmentComponent->InitializeOwner(PC);
}

void AInv_ProxyMeshActor::DelayedInitialization()
{
	FTimerDelegate TimerDelegate;
	TimerDelegate.BindUObject(this, &ThisClass::DelayedInitializeOwner);
	GetWorld()->GetTimerManager().SetTimerForNextTick(TimerDelegate);
}

