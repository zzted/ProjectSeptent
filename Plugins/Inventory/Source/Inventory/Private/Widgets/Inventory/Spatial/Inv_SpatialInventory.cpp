// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_SpatialInventory.h"

#include "Inventory.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Blueprint/WidgetTree.h"
#include "Components/Button.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "Components/WidgetSwitcher.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/GridSlots/Inv_EquippedGridSlot.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_EquippedSlottedItem.h"
#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"
#include "Widgets/ItemDescription/Inv_ItemDescription.h"

void UInv_SpatialInventory::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	Button_Equippables->OnClicked.AddDynamic(this, &UInv_SpatialInventory::ShowEquippables);
	Button_Consumables->OnClicked.AddDynamic(this, &UInv_SpatialInventory::ShowConsumables);
	Button_Craftables->OnClicked.AddDynamic(this, &UInv_SpatialInventory::ShowCraftables);
	
	Grid_Equippables->SetOwningCanvasPanel(CanvasPanel);
	Grid_Consumables->SetOwningCanvasPanel(CanvasPanel);
	Grid_Craftables->SetOwningCanvasPanel(CanvasPanel);
	ShowEquippables();
	
	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		if (UInv_EquippedGridSlot* EquippedGridSlot = Cast<UInv_EquippedGridSlot>(Widget))
		{
			EquippedGridSlots.Add(EquippedGridSlot);
			EquippedGridSlot->EquippedGridSlotClicked.AddDynamic(this, &UInv_SpatialInventory::EquippedGridSlotClicked);
		}
	});
}

void UInv_SpatialInventory::EquippedGridSlotClicked(UInv_EquippedGridSlot* EquippedGridSlot,
	const FGameplayTag& EquipmentTypeTag)
{
	// Check to see if we can equip the Hover Item
	if (!CanEquipHoverItem(EquippedGridSlot, EquipmentTypeTag)) return;
	// Create an equipped slotted item and add it to the equipped grid slot (call EquippedGridSlot->OnItemEquipped())
	UInv_EquippedSlottedItem* EquippedSlottedItem = 
		EquippedGridSlot->OnItemEquipped(GetHoverItem()->GetInventoryItem(), EquipmentTypeTag, GetTileSize());
	EquippedSlottedItem->OnEquippedSlottedItemClicked.AddDynamic(this, &UInv_SpatialInventory::EquippedSlottedItemClicked);
	
	// Inform the server that we've equipped an item (unequipping and item as well)
	UInv_InventoryComponent* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	check(IsValid(InventoryComponent));
	InventoryComponent->Server_EquipSlotClicked(GetHoverItem()->GetInventoryItem(), nullptr);
	
	if (GetOwningPlayer()->GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		InventoryComponent->OnItemEquipped.Broadcast(GetHoverItem()->GetInventoryItem());
	}
	
	// Clear the Hover Item
	Grid_Equippables->ClearHoverItem();
}

void UInv_SpatialInventory::EquippedSlottedItemClicked(UInv_EquippedSlottedItem* EquippedSlottedItem)
{
	const FGameplayTag& EquippedSlottedEquipmentTypeTag = EquippedSlottedItem->GetEquipmentTypeTag();
	
	// Remove the item description
	UInv_InventoryStatics::ItemUnhovered(GetOwningPlayer());
	if (IsValid(GetHoverItem()) && GetHoverItem()->IsStackable()) return; 
	
	// Get Item to Equip
	UInv_InventoryItem* ItemToEquip = IsValid(GetHoverItem()) ? GetHoverItem()->GetInventoryItem() : nullptr;
	
	// Get Item to Unequip
	UInv_InventoryItem* ItemToUnequip = EquippedSlottedItem->GetInventoryItem();
	
	// Get the equipped grid slot holding this item
	UInv_EquippedGridSlot* EquippedGridSlot = FindEquippedSlotWithEquippedItem(ItemToUnequip);
	
	// Check if the hover item has the correct tag
	if (IsValid(ItemToEquip) && !ItemToEquip->GetItemManifest().GetItemType().MatchesTag(EquippedSlottedEquipmentTypeTag)) return;
	
	// Clear the equipped grid slot of this item (set its inventory item to nullptr)
	ClearEquippedSlotOfItem(EquippedGridSlot);
	
	// Assign the previously equipped item as a hover item
	Grid_Equippables->AssignHoverItem(ItemToUnequip);
	
	// Remove of the equipped slotted item from the equipped grid slot 
	RemoveEquippedSlottedItem(EquippedSlottedItem);
	
	// Make a new equipped slotted item (from the item we held in HoverItem)
	CreateEquippedSlottedItem(EquippedSlottedEquipmentTypeTag, EquippedGridSlot, ItemToEquip);
	
	// Broadcast delegates for OnItemEquipped/OnItemUnequipped (from the IC)
	BroadcastSlotClickedDelegates(ItemToEquip, ItemToUnequip);
}

UInv_EquippedGridSlot* UInv_SpatialInventory::FindEquippedSlotWithEquippedItem(UInv_InventoryItem* EquippedItem) const
{
	const TObjectPtr<UInv_EquippedGridSlot>* FoundEquippedGridSlot = 
		EquippedGridSlots.FindByPredicate([EquippedItem](const UInv_EquippedGridSlot* GridSlot)
	{
		return GridSlot->GetInventoryItem().Get() == EquippedItem;
	});
	return FoundEquippedGridSlot ? *FoundEquippedGridSlot : nullptr;
}

void UInv_SpatialInventory::ClearEquippedSlotOfItem(UInv_EquippedGridSlot* EquippedGridSlot)
{
	if (IsValid(EquippedGridSlot))
	{
		EquippedGridSlot->SetEquippedSlottedItem(nullptr);
		EquippedGridSlot->SetInventoryItem(nullptr);
	}
}

void UInv_SpatialInventory::RemoveEquippedSlottedItem(UInv_EquippedSlottedItem* EquippedSlottedItem)
{
	if (!IsValid(EquippedSlottedItem)) return;
	
	// Unbind from OnEquipedSlottedItemClicked
	if (EquippedSlottedItem->OnEquippedSlottedItemClicked.IsAlreadyBound(this, &UInv_SpatialInventory::EquippedSlottedItemClicked))
	{
		EquippedSlottedItem->OnEquippedSlottedItemClicked.RemoveDynamic(this, &UInv_SpatialInventory::EquippedSlottedItemClicked);
	}
	EquippedSlottedItem->RemoveFromParent();
}

void UInv_SpatialInventory::CreateEquippedSlottedItem(const FGameplayTag& EquipmentTypeTag,
	UInv_EquippedGridSlot* EquippedGridSlot, UInv_InventoryItem* ItemToEquip)
{
	if (!IsValid(EquippedGridSlot)) return;
	UInv_EquippedSlottedItem* NewEquippedSlottedItem = EquippedGridSlot->OnItemEquipped(ItemToEquip, 
		EquipmentTypeTag, GetTileSize());
	if (IsValid(ItemToEquip)) 
		NewEquippedSlottedItem->OnEquippedSlottedItemClicked.AddDynamic(this, &UInv_SpatialInventory::EquippedSlottedItemClicked);
	EquippedGridSlot->SetEquippedSlottedItem(NewEquippedSlottedItem);
}

void UInv_SpatialInventory::BroadcastSlotClickedDelegates(UInv_InventoryItem* ItemToEquip,
	UInv_InventoryItem* ItemToUnequip) const
{
	UInv_InventoryComponent* InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	check(IsValid(InventoryComponent));
	InventoryComponent->Server_EquipSlotClicked(ItemToEquip, ItemToUnequip);
	
	if (GetOwningPlayer()->GetNetMode() != ENetMode::NM_DedicatedServer)
	{
		InventoryComponent->OnItemUnequipped.Broadcast(ItemToUnequip);
		InventoryComponent->OnItemEquipped.Broadcast(ItemToEquip);
	}
}

bool UInv_SpatialInventory::CanEquipHoverItem(const UInv_EquippedGridSlot* EquippedGridSlot,
                                              const FGameplayTag& EquipmentTypeTag)
{
	if (!IsValid(EquippedGridSlot) || EquippedGridSlot->GetInventoryItem().IsValid()) return false;
	const UInv_HoverItem* HoverItem = GetHoverItem();
	if (!IsValid(HoverItem)) return false;
	const UInv_InventoryItem* HeldItem = HoverItem->GetInventoryItem();
	return HasHoverItem() && IsValid(HeldItem) && 
		!HoverItem->IsStackable() && 
			HeldItem->GetItemManifest().GetItemCategory() == EInv_ItemCategory::Equippable &&
				HeldItem->GetItemManifest().GetItemType().MatchesTag(EquipmentTypeTag);
}

FReply UInv_SpatialInventory::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	// Mouse click on the current canvas panel will be considered outside the grid canvas since the later one is on top.
	ActiveGrid->DropItem();
	return FReply::Handled();
}

void UInv_SpatialInventory::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	if (!IsValid(ItemDescription)) return;
	SetItemDescriptionSizePosition(ItemDescription, CanvasPanel);
	SetEquippedItemDescriptionSizePosition(ItemDescription, EquippedItemDescription, CanvasPanel);
}

void UInv_SpatialInventory::SetItemDescriptionSizePosition(UInv_ItemDescription* Description,
	UCanvasPanel* Canvas) const
{
	UCanvasPanelSlot* ItemDescriptionCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(Description);
	if (!IsValid(ItemDescriptionCPS)) return;
	
	const FVector2D ItemDescriptionSize = Description->GetDesiredSize();
	ItemDescriptionCPS->SetSize(ItemDescriptionSize);
	const FVector2D ClampedPosition = UInv_WidgetUtils::GetClampedWidgetPosition(UInv_WidgetUtils::GetWidgetSize(Canvas), 
		ItemDescriptionSize, UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer()));
	ItemDescriptionCPS->SetPosition(ClampedPosition);
}

void UInv_SpatialInventory::SetEquippedItemDescriptionSizePosition(UInv_ItemDescription* Description,
	UInv_ItemDescription* EquippedDescription, UCanvasPanel* Canvas) const
{
	UCanvasPanelSlot* ItemDescriptionCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(Description);
	UCanvasPanelSlot* EquippedItemDescriptionCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(EquippedDescription);
	if (!IsValid(ItemDescriptionCPS) || !IsValid(EquippedItemDescriptionCPS)) return;
	const FVector2D ItemDescriptionSize = Description->GetDesiredSize();
	const FVector2D EquippedItemDescriptionSize = EquippedDescription->GetDesiredSize();
	FVector2D ClampedPosition = UInv_WidgetUtils::GetClampedWidgetPosition(UInv_WidgetUtils::GetWidgetSize(Canvas), 
		ItemDescriptionSize, UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer()));
	ClampedPosition.X -= ItemDescriptionSize.X;
	
	EquippedItemDescriptionCPS->SetSize(EquippedItemDescriptionSize);
	EquippedItemDescriptionCPS->SetPosition(ClampedPosition);
}

FInv_SlotAvailabilityResult UInv_SpatialInventory::HasRoomForItem(UInv_ItemComponent* ItemComponent) const
{
	switch (UInv_InventoryStatics::GetItemCategoryFromItemComp(ItemComponent))
	{
		case EInv_ItemCategory::Equippable:
			return Grid_Equippables->HasRoomForItem(ItemComponent);
		case EInv_ItemCategory::Consumable:
			return Grid_Consumables->HasRoomForItem(ItemComponent);
		case EInv_ItemCategory::Craftable:
			return Grid_Craftables->HasRoomForItem(ItemComponent);
		default:
			UE_LOG(LogInventory, Error, TEXT("ItemComponent doesn't have a valid Item Category."))
			return FInv_SlotAvailabilityResult();
	}
}

void UInv_SpatialInventory::OnItemHovered(UInv_InventoryItem* Item)
{
	const FInv_ItemManifest& Manifest = Item->GetItemManifest();
	UInv_ItemDescription* DescriptionWidget = GetItemDescription();
	DescriptionWidget->SetVisibility(ESlateVisibility::Collapsed);
	
	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(DescriptionTimer);
	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(EquippedDescriptionTimer);
	
	FTimerDelegate DescriptionTimerDelegate;
	DescriptionTimerDelegate.BindLambda([this, Item, &Manifest, DescriptionWidget]()
	{
		// Assimilate the manifest into the item description widget
		GetItemDescription()->SetVisibility(ESlateVisibility::HitTestInvisible); // Visible but no cursor hit 
		Manifest.AssimilateInventoryFragments(DescriptionWidget);
		
		// for the second item description, showing the equipped item of this type
		FTimerDelegate EquippedDescriptionTimerDelegate;
		EquippedDescriptionTimerDelegate.BindUObject(this, &UInv_SpatialInventory::ShowEquippedItemDescription, Item);
		GetOwningPlayer()->GetWorldTimerManager().SetTimer(EquippedDescriptionTimer, EquippedDescriptionTimerDelegate, EquippedDescriptionTimerDelay, false);
	});
	
	GetOwningPlayer()->GetWorldTimerManager().SetTimer(DescriptionTimer, DescriptionTimerDelegate, DescriptionTimerDelay, false);
}

void UInv_SpatialInventory::ShowEquippedItemDescription(UInv_InventoryItem* Item)
{
	const FInv_ItemManifest& Manifest = Item->GetItemManifest();
	const FInv_EquipmentFragment* EquipmentFragment = Manifest.GetFragmentOfType<FInv_EquipmentFragment>();
	if (!EquipmentFragment) return;
	const FGameplayTag& HoveredEquipmentTypeTag = EquipmentFragment->GetEquipmentTypeTag();
	TObjectPtr<UInv_EquippedGridSlot>* EquippedGridSlot = EquippedGridSlots.FindByPredicate([Item](const UInv_EquippedGridSlot* GridSlot)
	{
		return GridSlot->GetInventoryItem() == Item;
	});
	if (EquippedGridSlot != nullptr) return; // Hovered item already equipped, no need to show the equipped item description
	
	// It's not equipped, so find the equipped item with the same equipment type
	TObjectPtr<UInv_EquippedGridSlot>* FoundEquippedSlot = EquippedGridSlots.FindByPredicate([HoveredEquipmentTypeTag](const UInv_EquippedGridSlot* GridSlot)
	{
		UInv_InventoryItem* EquippedItem = GridSlot->GetInventoryItem().Get();
		return EquippedItem ? GridSlot->GetInventoryItem()->GetItemManifest().GetFragmentOfType<FInv_EquipmentFragment>()->GetEquipmentTypeTag() == HoveredEquipmentTypeTag : false;
	});
	if (!IsValid(*FoundEquippedSlot)) return; // No equipped item with the same equipment type
	
	UInv_InventoryItem* EquippedItem = (*FoundEquippedSlot)->GetInventoryItem().Get();
	if (!IsValid(EquippedItem)) return;
	
	const FInv_ItemManifest& EquippedItemManifest = EquippedItem->GetItemManifest();
	UInv_ItemDescription* EquippedItemDescriptionWidget = GetEquippedItemDescription();
	EquippedItemDescriptionWidget->Collapse();
	EquippedItemDescriptionWidget->SetVisibility(ESlateVisibility::HitTestInvisible);
	EquippedItemManifest.AssimilateInventoryFragments(EquippedItemDescriptionWidget);
}

void UInv_SpatialInventory::OnItemUnhovered()
{
	GetItemDescription()->SetVisibility(ESlateVisibility::Collapsed);
	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(DescriptionTimer);
	GetEquippedItemDescription()->SetVisibility(ESlateVisibility::Collapsed);
	GetOwningPlayer()->GetWorldTimerManager().ClearTimer(EquippedDescriptionTimer);
}

bool UInv_SpatialInventory::HasHoverItem() const
{
	if (Grid_Equippables->HasHoverItem() || Grid_Consumables->HasHoverItem() || Grid_Craftables->HasHoverItem()) return true;
	return false;
}

UInv_HoverItem* UInv_SpatialInventory::GetHoverItem()
{
	if (!ActiveGrid.IsValid()) return nullptr;
	return ActiveGrid->GetHoverItem();
}

float UInv_SpatialInventory::GetTileSize() const
{
	return Grid_Equippables->GetTileSize();
}

UInv_ItemDescription* UInv_SpatialInventory::GetItemDescription()
{
	if (!IsValid(ItemDescription))
	{
		ItemDescription = CreateWidget<UInv_ItemDescription>(GetOwningPlayer(), ItemDescriptionClass);
		CanvasPanel->AddChild(ItemDescription);
	}
	return ItemDescription;
}

UInv_ItemDescription* UInv_SpatialInventory::GetEquippedItemDescription()
{
	if (!IsValid(EquippedItemDescription))
	{
		EquippedItemDescription = CreateWidget<UInv_ItemDescription>(GetOwningPlayer(), EquippedItemDescriptionClass);
		CanvasPanel->AddChild(EquippedItemDescription);
	}
	return EquippedItemDescription;
}

void UInv_SpatialInventory::ShowEquippables()
{
	SetActiveGrid(Grid_Equippables, Button_Equippables);
}

void UInv_SpatialInventory::ShowConsumables()
{
	SetActiveGrid(Grid_Consumables, Button_Consumables);
}

void UInv_SpatialInventory::ShowCraftables()
{
	SetActiveGrid(Grid_Craftables, Button_Craftables);
}


void UInv_SpatialInventory::DisableButton(UButton* Button)
{
	Button_Equippables->SetIsEnabled(true);
	Button_Consumables->SetIsEnabled(true);
	Button_Craftables->SetIsEnabled(true);
	Button->SetIsEnabled(false);
}

void UInv_SpatialInventory::SetActiveGrid(UInv_InventoryGrid* Grid, UButton* Button)
{
	if (ActiveGrid.IsValid())
	{
		ActiveGrid->HideCursor();
		ActiveGrid->OnHide();
	}
	ActiveGrid = Grid;
	if (ActiveGrid.IsValid()) ActiveGrid->ShowCursor();
	DisableButton(Button);
	Switcher->SetActiveWidget(Grid);
}

