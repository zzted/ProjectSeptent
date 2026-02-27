#include "EquipmentManagement/Components/Inv_EquipmentComponent.h"

#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"
#include "GameFramework/Character.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"


void UInv_EquipmentComponent::SetOwningSkeletalMesh(USkeletalMeshComponent* OwningMesh)
{
	OwningSkeletalMesh = OwningMesh;
}

void UInv_EquipmentComponent::InitializeOwner(APlayerController* PlayerController)
{
	if (IsValid(PlayerController))
	{
		OwningPlayerController = PlayerController;
	}
	InitInventoryComponent();
}

void UInv_EquipmentComponent::BeginPlay()
{
	Super::BeginPlay();
	
	InitPlayerController();
}

void UInv_EquipmentComponent::InitPlayerController()
{
	OwningPlayerController = Cast<APlayerController>(GetOwner());
	if (OwningPlayerController.IsValid())
	{
		if (ACharacter* OwnerCharacter = Cast<ACharacter>(OwningPlayerController->GetPawn()); IsValid(OwnerCharacter))
		{
			OwningSkeletalMesh = OwnerCharacter->GetMesh();
			OnPossessedPawnChanged(nullptr, OwnerCharacter);
		}
		else
		{
			// In multiplayer, Pawn may not be possessed yet during BeginPlay, so we bind to OnPossessedPawnChanged to ensure safe initialization.
			OwningPlayerController->OnPossessedPawnChanged.AddDynamic(this, &UInv_EquipmentComponent::OnPossessedPawnChanged);
		}
	}
}

void UInv_EquipmentComponent::OnPossessedPawnChanged(APawn* OldPawn, APawn* NewPawn)
{
	if (const ACharacter* OwnerCharacter = Cast<ACharacter>(OwningPlayerController->GetPawn()); IsValid(OwnerCharacter))
	{
		OwningSkeletalMesh = OwnerCharacter->GetMesh();
	}
	InitInventoryComponent();
}

void UInv_EquipmentComponent::InitInventoryComponent()
{
	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(OwningPlayerController.Get());
	if (!InventoryComponent.IsValid()) return;
	if (!InventoryComponent->OnItemEquipped.IsAlreadyBound(this, &UInv_EquipmentComponent::OnItemEquipped))
	{
		InventoryComponent->OnItemEquipped.AddDynamic(this, &UInv_EquipmentComponent::OnItemEquipped);
	}
	if (!InventoryComponent->OnItemUnequipped.IsAlreadyBound(this, &UInv_EquipmentComponent::OnItemUnequipped))
	{
		InventoryComponent->OnItemUnequipped.AddDynamic(this, &UInv_EquipmentComponent::OnItemUnequipped);
	}
}

AInv_EquipActor* UInv_EquipmentComponent::SpawnEquippedActor(FInv_EquipmentFragment* EquipmentFragment,
	const FInv_ItemManifest& ItemManifest, USkeletalMeshComponent* AttachMesh)
{
	AInv_EquipActor* SpawnedActor = EquipmentFragment->SpawnAttachedActor(AttachMesh);
	SpawnedActor->SetEquipmentTypeTag(EquipmentFragment->GetEquipmentTypeTag());
	SpawnedActor->SetOwner(GetOwner());
	EquipmentFragment->SetEquippedActor(SpawnedActor);
	return SpawnedActor;
}

AInv_EquipActor* UInv_EquipmentComponent::FindEquippedActorByTag(const FGameplayTag& EquipmentTypeTag) const
{
	const TObjectPtr<AInv_EquipActor>* FoundActor = EquippedActors.FindByPredicate([&EquipmentTypeTag](const AInv_EquipActor* EquippedActor)
	{
		return EquippedActor->GetEquipmentTypeTag().MatchesTagExact(EquipmentTypeTag);
	});
	return FoundActor ? *FoundActor : nullptr;
}

void UInv_EquipmentComponent::RemoveEquippedActorByTag(const FGameplayTag& EquipmentTypeTag)
{
	AInv_EquipActor* EquippedActor = FindEquippedActorByTag(EquipmentTypeTag);
	if (IsValid(EquippedActor))
	{
		EquippedActors.Remove(EquippedActor);
		EquippedActor->Destroy();
	}
}

void UInv_EquipmentComponent::OnItemEquipped(UInv_InventoryItem* EquippedItem)
{
	if (!IsValid(EquippedItem)) return;
	if (!OwningPlayerController->HasAuthority()) return;
	FInv_ItemManifest& ItemManifest = EquippedItem->GetItemManifestMutable();
	FInv_EquipmentFragment* EquipmentFragment = ItemManifest.GetFragmentOfTypeMutable<FInv_EquipmentFragment>();
	if (!EquipmentFragment) return;
	if (!bIsProxy) EquipmentFragment->OnEquip(OwningPlayerController.Get());
	
	if (!OwningSkeletalMesh.IsValid()) return;
	AInv_EquipActor* SpawnedEquipActor = SpawnEquippedActor(EquipmentFragment, ItemManifest, OwningSkeletalMesh.Get());
	EquippedActors.Add(SpawnedEquipActor);
}

void UInv_EquipmentComponent::OnItemUnequipped(UInv_InventoryItem* UnequippedItem)
{
	if (!IsValid(UnequippedItem)) return;
	if (!OwningPlayerController->HasAuthority()) return;
	FInv_ItemManifest& ItemManifest = UnequippedItem->GetItemManifestMutable();
	FInv_EquipmentFragment* EquipmentFragment = ItemManifest.GetFragmentOfTypeMutable<FInv_EquipmentFragment>();
	if (!EquipmentFragment) return;
	
	if (!bIsProxy) EquipmentFragment->OnUnequip(OwningPlayerController.Get());
	
	RemoveEquippedActorByTag(EquipmentFragment->GetEquipmentTypeTag());
}




