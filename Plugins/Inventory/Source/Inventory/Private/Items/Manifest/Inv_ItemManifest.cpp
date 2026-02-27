#include "Items/Manifest/Inv_ItemManifest.h"

#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Kismet/GameplayStatics.h"
#include "Widgets/Composite/Inv_CompositeBase.h"

UInv_InventoryItem* FInv_ItemManifest::Manifest(UObject* NewOuter)
{
	UInv_InventoryItem* Item = NewObject<UInv_InventoryItem>(NewOuter, UInv_InventoryItem::StaticClass());
	Item->SetItemManifest(*this); // Create a copy of the item with the same manifest, in case the pickup need to destroy
	
	for (TInstancedStruct<FInv_ItemFragment>& Fragment : Item->GetItemManifestMutable().GetFragmentsMutable())
	{
		Fragment.GetMutable().Manifest();
	}
	ClearFragments();
	
	return Item;
}

void FInv_ItemManifest::AssimilateInventoryFragments(UInv_CompositeBase* Composite) const
{
	const TArray<const FInv_InventoryItemFragment*> InventoryItemFragments = 
		GetAllFragmentsOfType<FInv_InventoryItemFragment>();
	for (const FInv_InventoryItemFragment* Fragment : InventoryItemFragments)
	{
		/*
		 * For every fragment inheriting from InventoryItem fragment in the manifest, run the lambda function on the 
		 * composite root. ApplyFunction will automatically iterate the tree structure under the composite widget 
		 * until a leaf widget with a matching fragment tag is found the leaf will apply the assimilate 
		 * function of the corresponding fragment (leaf will run Function(this) on itself)
		 */
		Composite->ApplyFunction([Fragment](UInv_CompositeBase* Widget)
		{
			Fragment->Assimilate(Widget); // Should better named as Feed(Widget) since the data flows from fragment to widget.
		});
	}
}

void FInv_ItemManifest::SpawnPickupActor(const UObject* WorldContextObject, const FVector& SpawnLocation,
                                         const FRotator& SpawnRotation)
{
	if (!IsValid(PickupActorClass) || !IsValid(WorldContextObject)) return;
	
	AActor* SpawnedActor = WorldContextObject->GetWorld()->SpawnActor<AActor>(PickupActorClass, SpawnLocation, SpawnRotation);
	if (!IsValid(SpawnedActor)) return;
	
	// Set the item manifest, item category, item type, etc.
	UInv_ItemComponent* ItemComponent = SpawnedActor->FindComponentByClass<UInv_ItemComponent>();
	check(ItemComponent);
	
	// Init manifest by copying the current one
	ItemComponent->InitItemManifest(*this);
}

void FInv_ItemManifest::ClearFragments()
{
	for (TInstancedStruct<FInv_ItemFragment>& Fragment : Fragments)
	{
		Fragment.Reset();
	}
	Fragments.Empty();
}
