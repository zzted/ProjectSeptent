// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/GridSlots/Inv_EquippedGridSlot.h"

#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/Image.h"
#include "Components/Overlay.h"
#include "Components/OverlaySlot.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Fragments/Inv_FragmentTags.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_EquippedSlottedItem.h"

void UInv_EquippedGridSlot::NativeOnMouseEnter(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	if (!IsAvailable()) return;
	UInv_HoverItem* HoverItem = UInv_InventoryStatics::GetHoverItem(GetOwningPlayer());
	if (!IsValid(HoverItem)) return;
	
	if (HoverItem->GetItemType().MatchesTag(EquipmentTypeTag)) // More specific tag matches against a less specific one will succeed
	{
		SetOccupiedTexture();
		Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Collapsed);
	}
}

void UInv_EquippedGridSlot::NativeOnMouseLeave(const FPointerEvent& InMouseEvent)
{
	if (!IsAvailable()) return;
	UInv_HoverItem* HoverItem = UInv_InventoryStatics::GetHoverItem(GetOwningPlayer());
	if (!IsValid(HoverItem)) return;
	if (IsValid(EquippedSlottedItem)) return;
	
	if (HoverItem->GetItemType().MatchesTag(EquipmentTypeTag))
	{
		SetUnoccupiedTexture();
		Image_GrayedOutIcon->SetVisibility(ESlateVisibility::Visible);
	}
}

FReply UInv_EquippedGridSlot::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	EquippedGridSlotClicked.Broadcast(this, EquipmentTypeTag);
	return FReply::Handled();
}

UInv_EquippedSlottedItem* UInv_EquippedGridSlot::OnItemEquipped(UInv_InventoryItem* Item,
	const FGameplayTag& EquipmentTag, float TileSize)
{
	// check equipment type tag
	if (!EquipmentTypeTag.MatchesTagExact(EquipmentTag)) return nullptr;
	// get grid dimensions
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(Item, FragmentTags::GridFragment);
	if (!GridFragment) return nullptr;
	const FIntPoint Dimensions = GridFragment->GetGridSize();
	// calculate draw size for the equipped slotted item
	const FVector2D DrawSize = Dimensions * TileSize - GridFragment->GetGridPadding() * 2;
	// create the quipped slotted item widget
	EquippedSlottedItem = CreateWidget<UInv_EquippedSlottedItem>(GetOwningPlayer(), EquippedSlottedItemClass);
	// set slotted item's inventory item
	EquippedSlottedItem->SetInventoryItem(Item);
	// set slotted item's equipment type tag
	EquippedSlottedItem->SetEquipmentTypeTag(EquipmentTag);
	// hide the stack count widget on the slotted item
	EquippedSlottedItem->SetStackCount(0);
	// set the inventory item on this class (the equipped grid slot)
	SetInventoryItem(Item);
	// set the image brush on the equipped slotted item
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(Item, FragmentTags::IconFragment);
	if (!ImageFragment) return nullptr;
	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = DrawSize;
	EquippedSlottedItem->SetImageBrush(Brush);
	// add the slotted item as a child to this widget's overlay
	Overlay_Root->AddChildToOverlay(EquippedSlottedItem);
	FGeometry OverlayGeometry = Overlay_Root->GetCachedGeometry();
	auto OverlayPos = OverlayGeometry.GetAbsolutePosition();
	auto OverlaySize = OverlayGeometry.GetLocalSize();
	const float LeftPadding = OverlaySize.X * 0.5f - DrawSize.X * 0.5f;
	const float TopPadding = OverlaySize.Y * 0.5f - DrawSize.Y * 0.5f;
	
	UOverlaySlot* OverlaySlot = UWidgetLayoutLibrary::SlotAsOverlaySlot(EquippedSlottedItem);
	OverlaySlot->SetPadding(FMargin(LeftPadding, TopPadding));
	// return the equipped slotted item widget
	return EquippedSlottedItem;
}
