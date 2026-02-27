// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Inventory/Spatial/Inv_InventoryGrid.h"

#include "Inventory.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "Components/CanvasPanel.h"
#include "Components/CanvasPanelSlot.h"
#include "InventoryManagement/Components/Inv_InventoryComponent.h"
#include "InventoryManagement/Utils/Inv_InventoryStatics.h"
#include "Items/Inv_InventoryItem.h"
#include "Items/Components/Inv_ItemComponent.h"
#include "Items/Fragments/Inv_FragmentTags.h"
#include "Items/Fragments/Inv_ItemFragment.h"
#include "Widgets/Inventory/GridSlots/Inv_GridSlot.h"
#include "Widgets/Inventory/HoverItem/Inv_HoverItem.h"
#include "Widgets/Inventory/SlottedItems/Inv_SlottedItem.h"
#include "Widgets/ItemPopUp/Inv_ItemPopUp.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"

void UInv_InventoryGrid::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	ConstructGrid();
	
	InventoryComponent = UInv_InventoryStatics::GetInventoryComponent(GetOwningPlayer());
	InventoryComponent->OnItemAdded.AddDynamic(this, &UInv_InventoryGrid::AddItem);
	InventoryComponent->OnStackChange.AddDynamic(this, &UInv_InventoryGrid::AddStacks);
	InventoryComponent->OnInventoryMenuToggled.AddDynamic(this, &UInv_InventoryGrid::OnInventoryMenuToggled);
}

void UInv_InventoryGrid::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	
	const FVector2D CanvasPosition = UInv_WidgetUtils::GetWidgetPosition(CanvasPanel);
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
	
	bLastMouseWithinCanvas = bMouseWithinCanvas;
	bMouseWithinCanvas = UInv_WidgetUtils::IsWithinWidgetBounds(CanvasPosition, UInv_WidgetUtils::GetWidgetSize(CanvasPanel), MousePosition);
	
	if (!bMouseWithinCanvas && bLastMouseWithinCanvas) // Mouse left canvas
	{
		UnhighlightSlots(LastHighlightedIndex, LastHighlightedDimensions);
		return;
	} 
	
	UpdateTileParameters(CanvasPosition, MousePosition);
}

void UInv_InventoryGrid::ConstructGrid()
{
	GridSlots.Reserve(Rows * Columns);
	
	for (int32 i = 0; i < Rows; ++i)
	{
		for (int32 j = 0; j < Columns; ++j)
		{
			UInv_GridSlot* GridSlot = CreateWidget<UInv_GridSlot>(this, GridSlotClass);
			CanvasPanel->AddChild(GridSlot);
			const FIntPoint TilePosition(j, i);
			GridSlot->SetTileIndex(UInv_WidgetUtils::GetIndexFromPosition(TilePosition, Columns));
			
			UCanvasPanelSlot* GridCPS = UWidgetLayoutLibrary::SlotAsCanvasSlot(GridSlot);
			GridCPS->SetSize(FVector2D(TileSize));
			GridCPS->SetPosition(TilePosition * TileSize);
			
			GridSlots.Add(GridSlot);
			GridSlot->GridSlotClicked.AddDynamic(this, &UInv_InventoryGrid::OnGridSlotClicked);
			GridSlot->GridSlotHovered.AddDynamic(this, &UInv_InventoryGrid::OnGridSlotHovered);
			GridSlot->GridSlotUnhovered.AddDynamic(this, &UInv_InventoryGrid::OnGridSlotUnhovered);
		}
	}
}

void UInv_InventoryGrid::OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (!IsValid(HoverItem)) return;
	if (!GridSlots.IsValidIndex(ItemDropIndex)) return;
	// Clicked on a location that has a valid item.
	if (CurrentSpaceQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentSpaceQueryResult.UpperLeftIndex))
	{
		OnSlottedItemClicked(CurrentSpaceQueryResult.UpperLeftIndex, MouseEvent);
		return;
	}
	
	if (!IsInGridBounds(ItemDropIndex, HoverItem->GetGridDimensions())) return;
	TObjectPtr<UInv_GridSlot> GridSlot = GridSlots[ItemDropIndex];
	if (!GridSlot->GetInventoryItem().IsValid())
	{
		// Put item down at this index
		AddItemAtIndex(HoverItem->GetInventoryItem(), ItemDropIndex, HoverItem->IsStackable(), HoverItem->GetStackCount());
		UpdateGridSlots(HoverItem->GetInventoryItem(), ItemDropIndex, HoverItem->IsStackable(), HoverItem->GetStackCount());
		ClearHoverItem();
	}
}

void UInv_InventoryGrid::ClearHoverItem()
{
	if (!IsValid(HoverItem)) return;
	HoverItem->SetInventoryItem(nullptr);
	HoverItem->SetIsStackable(false);
	HoverItem->SetStackCount(0);
	HoverItem->SetPreviousGridIndex(INDEX_NONE);
	HoverItem->SetImageBrush(FSlateNoResource());
	HoverItem->RemoveFromParent();
	HoverItem = nullptr;
	
	// Show Mouse Cursor
	ShowCursor();
}


UUserWidget* UInv_InventoryGrid::GetVisibleCursorWidget()
{
	if (!IsValid(GetOwningPlayer())) return nullptr;
	if (!IsValid(VisibleCursorWidget))
	{
		VisibleCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), VisibleCursorWidgetClass);
	}
	return VisibleCursorWidget;
}

UUserWidget* UInv_InventoryGrid::GetHiddenCursorWidget()
{
	if (!IsValid(GetOwningPlayer())) return nullptr;
	if (!IsValid(HiddenCursorWidget))
	{
		HiddenCursorWidget = CreateWidget<UUserWidget>(GetOwningPlayer(), HiddenCursorWidgetClass);
	}
	return HiddenCursorWidget;
}

void UInv_InventoryGrid::ShowCursor()
{
	if (!IsValid(GetOwningPlayer())) return;
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, GetVisibleCursorWidget());
}

void UInv_InventoryGrid::HideCursor()
{
	if (!IsValid(GetOwningPlayer())) return;
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, GetHiddenCursorWidget());
}

void UInv_InventoryGrid::SetOwningCanvasPanel(UCanvasPanel* OwningCanvas)
{
	OwningCanvasPanel = OwningCanvas;
}

void UInv_InventoryGrid::OnGridSlotHovered(const int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem)) return;
	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->IsAvailable())
	{
		GridSlot->SetOccupiedTexture();
	}
}

void UInv_InventoryGrid::OnGridSlotUnhovered(const int32 GridIndex, const FPointerEvent& MouseEvent)
{
	if (IsValid(HoverItem)) return;
	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	if (GridSlot->IsAvailable())
	{
		GridSlot->SetUnoccupiedTexture();
	}
}

void UInv_InventoryGrid::OnPopUpMenuSplit(int32 SplitAmount, int32 GridIndex)
{
	UInv_InventoryItem* InventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();
	if (!IsValid(InventoryItem)) return;
	if (!InventoryItem->IsStackable()) return;
	
	const int32 UpperLeftIndex = GridSlots[GridIndex]->GetUpperLeftIndex();
	UInv_GridSlot* UpperLeftSlot = GridSlots[UpperLeftIndex];
	const int32 StackCount = UpperLeftSlot->GetStackCount();
	const int32 NewStackCount = StackCount - SplitAmount;
	
	UpperLeftSlot->SetStackCount(NewStackCount);
	SlottedItems.FindChecked(UpperLeftIndex)->SetStackCount(NewStackCount);
	CreateHoverItemWidget(UpperLeftIndex, InventoryItem);
	HoverItem->SetStackCount(SplitAmount);
}

void UInv_InventoryGrid::OnPopUpMenuDrop(int32 GridIndex)
{
	UInv_InventoryItem* InventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();
	if (!IsValid(InventoryItem)) return;
	CreateHoverItemWidget(GridIndex, InventoryItem);
	RemoveItemFromGrid(GridIndex, InventoryItem);
	DropItem();
}

void UInv_InventoryGrid::OnPopUpMenuConsume(int32 GridIndex)
{
	UInv_InventoryItem* InventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();
	if (!IsValid(InventoryItem)) return;
	const int32 UpperLeftIndex = GridSlots[GridIndex]->GetUpperLeftIndex();
	UInv_GridSlot* UpperLeftSlot = GridSlots[UpperLeftIndex];
	const int32 NewStackCount = UpperLeftSlot->GetStackCount() - 1;
	UpperLeftSlot->SetStackCount(NewStackCount);
	SlottedItems.FindChecked(UpperLeftIndex)->SetStackCount(NewStackCount);
	
	// Tell the server we're consuming an item
	InventoryComponent->Server_ConsumeItem(InventoryItem);
	
	if (NewStackCount <= 0)
	{
		RemoveItemFromGrid(UpperLeftIndex, InventoryItem);
	}
}

void UInv_InventoryGrid::OnInventoryMenuToggled(bool bIsOpen)
{
	if (!bIsOpen)
	{
		PutBackHoverItem();
	}
}

void UInv_InventoryGrid::UpdateTileParameters(const FVector2D& CanvasPosition, const FVector2D& MousePosition)
{
	// If mouse not in canvas panel, return
	if (!bMouseWithinCanvas) return;
	
	// Calculate the tile quadrant, tile index and coordinates
	const FIntPoint HoveredTileCoordinates 
	{
		static_cast<int32>(FMath::FloorToInt((MousePosition.X - CanvasPosition.X) / TileSize)),
		static_cast<int32>(FMath::FloorToInt((MousePosition.Y - CanvasPosition.Y) / TileSize))
	};
	LastTileParameters = TileParameters;
	TileParameters.TileCoordinates = HoveredTileCoordinates;
	TileParameters.TileIndex = UInv_WidgetUtils::GetIndexFromPosition(HoveredTileCoordinates, Columns);
	TileParameters.TileQuadrant = CalculateTileQuadrant(CanvasPosition, MousePosition);
	
	// Handle highlight/unhighlight of the grid slots
	OnTileParametersUpdate(TileParameters);
}

void UInv_InventoryGrid::OnTileParametersUpdate(const FInv_TileParameters& NewTileParameters)
{
	if (!IsValid(HoverItem)) return;
	// Get Hover Item dimensions
	const FIntPoint Dimensions = HoverItem->GetGridDimensions();
	// Calculate the starting coordinate for highlighting
	const FIntPoint StartingCoordinates = CalculateStartingCoordinates(LastTileParameters.TileCoordinates, Dimensions, LastTileParameters.TileQuadrant);
	ItemDropIndex = UInv_WidgetUtils::GetIndexFromPosition(StartingCoordinates, Columns);
	// Check Hover Position and get CurrentSpaceQueryResult
	CurrentSpaceQueryResult = CheckHoverPosition(StartingCoordinates, Dimensions);
	// No Item in the way
	if (CurrentSpaceQueryResult.bHasSpace)
	{
		HighlightSlots(ItemDropIndex, Dimensions);
		return;
	}
	// One item in the way
	UnhighlightSlots(LastHighlightedIndex, LastHighlightedDimensions);
	if (CurrentSpaceQueryResult.ValidItem.IsValid() && GridSlots.IsValidIndex(CurrentSpaceQueryResult.UpperLeftIndex))
	{
		// Single item in the space, swap and add stacks
		const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(CurrentSpaceQueryResult.ValidItem.Get(), FragmentTags::GridFragment);
		if (!GridFragment) return;
		ChangeHoverHighlightType(CurrentSpaceQueryResult.UpperLeftIndex, GridFragment->GetGridSize(), EInv_GridSlotState::GrayedOut);
	}
}

FInv_SpaceQueryResult UInv_InventoryGrid::CheckHoverPosition(const FIntPoint& Position, const FIntPoint& Dimensions)
{
	FInv_SpaceQueryResult Result;
	// Are the dimensions within the grid bounds?
	if (!IsInGridBounds(ItemDropIndex, Dimensions)) return Result;
	
	Result.bHasSpace = true;
	// If more than one of the indices is occupied with items, are they all having the same upper left index?
	TSet<int32> OccupiedUpperLeftIndices;
	UInv_InventoryStatics::ForEach2D(GridSlots, ItemDropIndex, Dimensions, Columns, [&](const UInv_GridSlot* GridSlot)
	{
		if (GridSlot->GetInventoryItem().IsValid())
		{
			OccupiedUpperLeftIndices.Add(GridSlot->GetUpperLeftIndex());
			Result.bHasSpace = false;
		}
	});
	// If so, is there only one item in the way (can we swap)?
	if (OccupiedUpperLeftIndices.Num() == 1)
	{
		const int32 Index = *OccupiedUpperLeftIndices.CreateConstIterator();
		Result.ValidItem = GridSlots[Index]->GetInventoryItem().Get();
		Result.UpperLeftIndex = Index;
	}
	
	return Result;
}

void UInv_InventoryGrid::HighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	if (!bMouseWithinCanvas) return;
	UnhighlightSlots(LastHighlightedIndex, LastHighlightedDimensions);
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInv_GridSlot* GridSlot)
	{
		GridSlot->SetOccupiedTexture();
	});
	LastHighlightedIndex = Index;
	LastHighlightedDimensions = Dimensions;
}

void UInv_InventoryGrid::UnhighlightSlots(const int32 Index, const FIntPoint& Dimensions)
{
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInv_GridSlot* GridSlot)
	{
		if (GridSlot->IsAvailable())
		{
			GridSlot->SetUnoccupiedTexture();
		}
		else
		{
			GridSlot->SetOccupiedTexture();
		}
	});
}

void UInv_InventoryGrid::ChangeHoverHighlightType(const int32 Index, const FIntPoint& Dimensions,
	EInv_GridSlotState GridSlotState)
{
	UnhighlightSlots(LastHighlightedIndex, LastHighlightedDimensions);
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [State = GridSlotState](UInv_GridSlot* GridSlot)
	{
		switch (State)
		{
		case EInv_GridSlotState::Occupied:
			GridSlot->SetOccupiedTexture();
			break;
		case EInv_GridSlotState::Unoccupied:
			GridSlot->SetUnoccupiedTexture();
			break;
		case EInv_GridSlotState::GrayedOut:
			GridSlot->SetGrayedOutTexture();
			break;
		case EInv_GridSlotState::Selected:
			GridSlot->SetSelectedTexture();
			break;
		}
	});
	LastHighlightedIndex = Index;
	LastHighlightedDimensions = Dimensions;
}


FIntPoint UInv_InventoryGrid::CalculateStartingCoordinates(const FIntPoint& Coordinate, const FIntPoint& Dimensions,
                                                           const EInv_TileQuadrant Quadrant)
{
	const int32 HasEvenWidth = Dimensions.X % 2 == 0;
	const int32 HasEvenHeight = Dimensions.Y % 2 == 0;
	
	FIntPoint StartingCoordinates;
	switch (Quadrant)
	{
		case EInv_TileQuadrant::TopLeft: 
			StartingCoordinates.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
			StartingCoordinates.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
			break;
		case EInv_TileQuadrant::TopRight: 
			StartingCoordinates.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
			StartingCoordinates.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y);
			break;
		case EInv_TileQuadrant::BottomLeft: 
			StartingCoordinates.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X);
			StartingCoordinates.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
			break;
		case EInv_TileQuadrant::BottomRight: 
			StartingCoordinates.X = Coordinate.X - FMath::FloorToInt(0.5f * Dimensions.X) + HasEvenWidth;
			StartingCoordinates.Y = Coordinate.Y - FMath::FloorToInt(0.5f * Dimensions.Y) + HasEvenHeight;
			break;
		default:
			UE_LOG(LogInventory, Error, TEXT("Invalid Quadrant."));
			return FIntPoint(-1, -1);
	}
	return StartingCoordinates;
}



EInv_TileQuadrant UInv_InventoryGrid::CalculateTileQuadrant(const FVector2D& CanvasPosition,
                                                            const FVector2D& MousePosition)
{
	// Calculate relative position within the current tile
	const float TileLocalX = FMath::Fmod(MousePosition.X - CanvasPosition.X, TileSize);
	const float TileLocalY = FMath::Fmod(MousePosition.Y - CanvasPosition.Y, TileSize);
	
	// Determine which quadrant the mouse is in
	EInv_TileQuadrant HoveredTileQuadrant;
	if (TileLocalX < TileSize / 2)
	{
		if (TileLocalY < TileSize / 2) 
			HoveredTileQuadrant = EInv_TileQuadrant::TopLeft;
		else 
			HoveredTileQuadrant = EInv_TileQuadrant::BottomLeft;
	}
	else
	{
		if (TileLocalY < TileSize / 2) 
			HoveredTileQuadrant = EInv_TileQuadrant::TopRight;
		else 
			HoveredTileQuadrant = EInv_TileQuadrant::BottomRight;
	}
	return HoveredTileQuadrant;
}

void UInv_InventoryGrid::AddItem(UInv_InventoryItem* InventoryItem)
{
	if (!MatchesCategory(InventoryItem)) return;

	const FInv_SlotAvailabilityResult Result = HasRoomForItem(InventoryItem);
	
	AddItemToIndices(Result, InventoryItem);
}



FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_ItemComponent* ItemComponent)
{
	return HasRoomForItem(ItemComponent->GetItemManifest());
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const UInv_InventoryItem* InventoryItem, const int32 StackAmountOverride)
{
	return HasRoomForItem(InventoryItem->GetItemManifest(), StackAmountOverride);
}

FInv_SlotAvailabilityResult UInv_InventoryGrid::HasRoomForItem(const FInv_ItemManifest& Manifest, const int32 StackAmountOverride)
{
	FInv_SlotAvailabilityResult Result;
	// Determine if the item is stackable
	const FInv_StackableFragment* StackableFragment = Manifest.GetFragmentOfType<FInv_StackableFragment>();
	Result.bStackable = StackableFragment != nullptr;
	
	// Determine how many stacks to add
	const int32 MaxStackSize = Result.bStackable ? StackableFragment->GetMaxStackSize() : 1;
	int32 AmountToFill = Result.bStackable ? StackableFragment->GetStackCount() : 1;
	if (StackAmountOverride != -1 && Result.bStackable) AmountToFill = StackAmountOverride;
	
	TSet<int32> OccupiedIndices;
	// For each Grid slot:
	for (const TObjectPtr<UInv_GridSlot>& GridSlot : GridSlots)
	{
		
		// If we don't have anymore to fill, break out the loop early
		if (AmountToFill == 0) break;
		
		// Is this index claimed yet?
		if (OccupiedIndices.Contains(GridSlot->GetTileIndex())) continue;
		
		// Within the grid bounds?
		const FInv_GridFragment* GridFragment = Manifest.GetFragmentOfType<FInv_GridFragment>();
		const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
		if (!IsInGridBounds(GridSlot->GetTileIndex(), Dimensions)) continue;
		
		// Can the item fit here? (i.e., is it out of grid bounds?)
		TSet<int32> TentativeOccupiedIndices;
		if (!HasRoomAtIndex(GridSlot, Dimensions, OccupiedIndices,
			TentativeOccupiedIndices, Manifest.GetItemType(), MaxStackSize)) continue;
		
		// How much to fill?
		const int32 AmountToFillInSlot = DetermineFillAmountForSlot(Result.bStackable, MaxStackSize, AmountToFill, GridSlot);
		if (AmountToFillInSlot == 0) continue;
		
		OccupiedIndices.Append(TentativeOccupiedIndices);
		
		// Update the amount left to fill
		Result.TotalRoomToFill += AmountToFillInSlot;
		Result.SlotAvailabilities.Emplace(
			GridSlot->GetInventoryItem().IsValid() ? GridSlot->GetUpperLeftIndex() : GridSlot->GetTileIndex(),
			Result.bStackable ? AmountToFillInSlot : 0, 
			GridSlot->GetInventoryItem().IsValid()
			);
		
		// How much is the Remainder?
		AmountToFill -= AmountToFillInSlot;
		Result.Remainder = AmountToFill;
		if (Result.Remainder == 0) return Result;
	}
	
	return Result;
}

bool UInv_InventoryGrid::IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const
{
	if (StartIndex < 0 || StartIndex >= GridSlots.Num()) return false;
	const int32 EndRow = StartIndex / Columns + ItemDimensions.Y - 1;
	const int32 EndColumn = StartIndex % Columns + ItemDimensions.X - 1;
	return EndRow < Rows && EndColumn < Columns;
}

int32 UInv_InventoryGrid::DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize,
	const int32 AmountToFill, const UInv_GridSlot* GridSlot)
{
	int32 CurrentSlotStackCount = GridSlot->GetStackCount();
	if (const int32 UpperLeftIndex = GridSlot->GetUpperLeftIndex(); UpperLeftIndex != INDEX_NONE)
	{
		CurrentSlotStackCount = GridSlots[UpperLeftIndex]->GetStackCount();
	}
	const int32 RoomInSlot = MaxStackSize - CurrentSlotStackCount;
	return bStackable ? FMath::Min(AmountToFill, RoomInSlot) : 1;
}

void UInv_InventoryGrid::AddStacks(const FInv_SlotAvailabilityResult& Result)
{
	if (!MatchesCategory(Result.Item.Get())) return;
	
	for (const FInv_SlotAvailability& SlotAvailability : Result.SlotAvailabilities)
	{
		if (SlotAvailability.bItemAtIndex)
		{
			const TObjectPtr<UInv_GridSlot>& GridSlot = GridSlots[SlotAvailability.Index];
			const TObjectPtr<UInv_SlottedItem>& SlottedItem = SlottedItems.FindChecked(SlotAvailability.Index);
			// Slotted item only control the item widget
			SlottedItem->SetStackCount(GridSlot->GetStackCount() + SlotAvailability.AmountToFill);
			// GridSlot is the slot widget item seats in, has the stack count info in it
			GridSlot->SetStackCount(GridSlot->GetStackCount() + SlotAvailability.AmountToFill);
		}
		else
		{
			AddItemAtIndex(Result.Item.Get(), SlotAvailability.Index, Result.bStackable, SlotAvailability.AmountToFill);
			UpdateGridSlots(Result.Item.Get(), SlotAvailability.Index, Result.bStackable, SlotAvailability.AmountToFill);
		}
	}
}

bool UInv_InventoryGrid::HasRoomAtIndex(const UInv_GridSlot* GridSlot, const FIntPoint& Dimensions, 
                                        const TSet<int32>& OccupiedIndices, TSet<int32>& TentativelyOccupiedIndices, 
                                        const FGameplayTag& ItemType, const int32 MaxStackSize)
{
	bool bHasRoomAtIndex = true;
	// Is there room at this index? (i.e., are there other items in the way?)
	UInv_InventoryStatics::ForEach2D(GridSlots, GridSlot->GetTileIndex(), Dimensions, Columns, 
		[&](const UInv_GridSlot* SubGridSlot)
		{
			if (CheckSlotConstrains(GridSlot, SubGridSlot, OccupiedIndices, ItemType, MaxStackSize))
			{
				TentativelyOccupiedIndices.Add(SubGridSlot->GetTileIndex());
			}
			else
			{
				// Any of the subgrid not available means not available;
				bHasRoomAtIndex = false;
			}
		});
	
	return bHasRoomAtIndex;
}

bool UInv_InventoryGrid::CheckSlotConstrains(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot, 
	const TSet<int32>& OccupiedIndices, const FGameplayTag& ItemType, const int32 MaxStackSize)
{
	// Index claimed? 
	if (OccupiedIndices.Contains(SubGridSlot->GetTileIndex())) return false;
	// Has valid item?
	if (!SubGridSlot->GetInventoryItem().IsValid()) return true;
	// Is this Grid slot an upper left slot?
	if (SubGridSlot->GetUpperLeftIndex() != GridSlot->GetTileIndex()) return false;
	// If so, is this a stackable item?
	if (!SubGridSlot->GetInventoryItem().Get()->IsStackable()) return false;
	// Is this item the same type as the item we're trying to add?
	if (!SubGridSlot->GetInventoryItem().Get()->GetItemManifest().GetItemType().MatchesTagExact(ItemType)) return false;
	// If stackable, is this slot at max stack size already?
	if (GridSlot->GetStackCount() >= MaxStackSize) return false;
	
	return true;
}



void UInv_InventoryGrid::AddItemToIndices(const FInv_SlotAvailabilityResult& Result,
                                          UInv_InventoryItem* NewInventoryItem)
{
	for (const FInv_SlotAvailability& SlotAvailability : Result.SlotAvailabilities)
	{
		AddItemAtIndex(NewInventoryItem, SlotAvailability.Index, Result.bStackable, SlotAvailability.AmountToFill);
		UpdateGridSlots(NewInventoryItem, SlotAvailability.Index, Result.bStackable, SlotAvailability.AmountToFill);
	}
}

void UInv_InventoryGrid::AddItemAtIndex(UInv_InventoryItem* NewInventoryItem, int32 Index, const bool bStackable,
                                        const int32 StackAmount)
{
	// Get Grid Fragment so we know how many grid spaces the item takes
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(NewInventoryItem, FragmentTags::GridFragment);
	// Get Image Fragment so we have the icon to display
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(NewInventoryItem, FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment) return;
	// Create a widget to add to the grid
	UInv_SlottedItem* SlottedItem = CreateSlottedItem(NewInventoryItem, Index, GridFragment, ImageFragment, bStackable, StackAmount);
	// Add slotted item to the canvas panel
	AddSlottedItemToCanvas(Index, GridFragment, SlottedItem);
	// Store the new widget in a container
	SlottedItems.Add(Index, SlottedItem);
}

UInv_SlottedItem* UInv_InventoryGrid::CreateSlottedItem(UInv_InventoryItem* NewInventoryItem,
                                                        const int32 Index, const FInv_GridFragment* GridFragment, 
                                                        const FInv_ImageFragment* ImageFragment, const bool bStackable, 
                                                        const int32 StackAmount) const
{
	UInv_SlottedItem* SlottedItem = CreateWidget<UInv_SlottedItem>(GetOwningPlayer(), SlottedItemClass);
	SlottedItem->SetInventoryItem(NewInventoryItem);
	SetSlottedItemImage(SlottedItem, GridFragment, ImageFragment);
	SlottedItem->SetGridIndex(Index);
	SlottedItem->SetIsStackable(bStackable);
	const int32 StackCount = bStackable ? StackAmount : 0;
	SlottedItem->SetStackCount(StackCount);
	SlottedItem->OnSlottedItemClicked.AddDynamic(this, &ThisClass::OnSlottedItemClicked);
	return SlottedItem;
}

void UInv_InventoryGrid::OnSlottedItemClicked(int32 GridIndex, const FPointerEvent& MouseEvent)
{
	UInv_InventoryStatics::ItemUnhovered(GetOwningPlayer());
	check(GridSlots.IsValidIndex(GridIndex));
	UInv_InventoryItem* ClickedInventoryItem = GridSlots[GridIndex]->GetInventoryItem().Get();
	if (!IsValid(HoverItem) && MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		CreateHoverItemWidget(GridIndex, ClickedInventoryItem);
		RemoveItemFromGrid(GridIndex, ClickedInventoryItem);
		return;
	}
	
	if (MouseEvent.GetEffectingButton() == EKeys::RightMouseButton)
	{
		CreateItemPopUp(GridIndex);
		return;
	}
	
	// Are the hovered item and clicked inventory item the same type and stackable?
	if (IsSameStackable(ClickedInventoryItem))
	{
		const int32 ClickedStackCount = GridSlots[GridIndex]->GetStackCount();
		const FInv_StackableFragment* StackableFragment = ClickedInventoryItem->GetItemManifest().GetFragmentOfType<FInv_StackableFragment>();
		const int32 MaxStackSize = StackableFragment->GetMaxStackSize();
		const int32 RoomInClickedSlot = MaxStackSize - ClickedStackCount;
		const int32 HoveredStackCount = HoverItem->GetStackCount();
		// Should we swap stack counts? (Room in the clicked slot is 0 && hovered stack count < max stack size)
		if (RoomInClickedSlot == 0 && HoveredStackCount < MaxStackSize)
		{
			SwapStackCounts(GridIndex, ClickedStackCount, HoveredStackCount);
			return;
		}
		// Should we consume the hover item's stacks? (Room in clicked slot >= HoveredStackCount)
		if (RoomInClickedSlot >= HoveredStackCount)
		{
			ConsumeHoverItemStack(GridIndex, ClickedStackCount, HoveredStackCount);
			return;
		}
		// Should we fill in the stacks of the clicked item? (not consume the hover item)
		if (RoomInClickedSlot < HoveredStackCount)
		{
			FillInStack(GridIndex, RoomInClickedSlot, HoveredStackCount - RoomInClickedSlot);
			return;
		}
		// Clicked slot is full - do nothing (play a sound)
		if (RoomInClickedSlot == 0)
		{
			return;
		}
	}
	
	// Make sure we can swap with only one valid item
	if (CurrentSpaceQueryResult.ValidItem.IsValid())
	{
		// Swap with the hover item.
		SwapWithHoverItem(GridIndex, ClickedInventoryItem);
	}
}


void UInv_InventoryGrid::CreateItemPopUp(const int32 GridIndex)
{
	UInv_InventoryItem* RightClickedItem = GridSlots[GridIndex]->GetInventoryItem().Get();
	if (!IsValid(RightClickedItem)) return;
	if (IsValid(GridSlots[GridIndex]->GetItemPopUp())) return ;
	
	ItemPopUp = CreateWidget<UInv_ItemPopUp>(this, ItemPopUpClass);
	GridSlots[GridIndex]->SetItemPopUp(ItemPopUp);
	
	OwningCanvasPanel->AddChild(ItemPopUp);
	UCanvasPanelSlot* CanvasPanelSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(ItemPopUp);
	const FVector2D MousePosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
	CanvasPanelSlot->SetPosition(MousePosition - ItemPopUpOffset);
	CanvasPanelSlot->SetSize(ItemPopUp->GetSizeBoxSize());
	
	const int32 SliderMax = GridSlots[GridIndex]->GetStackCount() - 1;
	if (RightClickedItem->IsStackable() && SliderMax > 0)
	{
		ItemPopUp->OnSplit.BindDynamic(this, &ThisClass::OnPopUpMenuSplit);
		ItemPopUp->SetSliderParams(SliderMax, FMath::Max(1, GridSlots[GridIndex]->GetStackCount() / 2));
	}
	else
	{
		ItemPopUp->CollapseSplitButton();
	}
	
	ItemPopUp->OnDrop.BindDynamic(this, &ThisClass::OnPopUpMenuDrop);
	
	if (RightClickedItem->IsConsumable())
	{
		ItemPopUp->OnConsume.BindDynamic(this, &ThisClass::OnPopUpMenuConsume);
	}
	else
	{
		ItemPopUp->CollapseConsumeButton();
	}
}

void UInv_InventoryGrid::DropItem()
{
	if (!IsValid(HoverItem)) return;
	if (!IsValid(HoverItem->GetInventoryItem())) return;
	
	// Tell the server to actually drop the item
	InventoryComponent->Server_DropItem(HoverItem->GetInventoryItem(), HoverItem->GetStackCount());
	
	ClearHoverItem();
}

bool UInv_InventoryGrid::HasHoverItem() const
{
	return IsValid(HoverItem);
}

UInv_HoverItem* UInv_InventoryGrid::GetHoverItem()
{
	return HoverItem;
}


bool UInv_InventoryGrid::IsSameStackable(const UInv_InventoryItem* ClickedInventoryItem) const
{
	const bool bIsSameItem = ClickedInventoryItem == HoverItem->GetInventoryItem();
	const bool bIsStackable = ClickedInventoryItem->IsStackable();
	return bIsSameItem && bIsStackable && HoverItem->GetItemType().MatchesTagExact(ClickedInventoryItem->GetItemManifest().GetItemType());
}

void UInv_InventoryGrid::SwapWithHoverItem(const int32 GridIndex, UInv_InventoryItem* ClickedInventoryItem)
{
	if (!IsValid(HoverItem)) return;
	UInv_InventoryItem* TempInventoryItem = HoverItem->GetInventoryItem();
	const int32 TempStackCount = HoverItem->GetStackCount();
	const bool bTempIsStackable = HoverItem->IsStackable();
	
	// Keep the same previous grid index
	CreateHoverItemWidget(GridIndex, ClickedInventoryItem);
	RemoveItemFromGrid(GridIndex, ClickedInventoryItem);
	AddItemAtIndex(TempInventoryItem, ItemDropIndex, bTempIsStackable, TempStackCount);
	UpdateGridSlots(TempInventoryItem, ItemDropIndex, bTempIsStackable, TempStackCount);
}

void UInv_InventoryGrid::SwapStackCounts(const int32 GridIndex, const int32 ClickedStackCount,
	const int32 HoverStackCount)
{
	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(GridIndex);
	GridSlot->SetStackCount(HoverStackCount);
	ClickedSlottedItem->SetStackCount(HoverStackCount);
	HoverItem->SetStackCount(ClickedStackCount);
}

void UInv_InventoryGrid::ConsumeHoverItemStack(const int32 GridIndex, const int32 ClickedStackCount,
	const int32 HoverStackCount)
{
	GridSlots[GridIndex]->SetStackCount(HoverStackCount + ClickedStackCount);
	SlottedItems.FindChecked(GridIndex)->SetStackCount(HoverStackCount + ClickedStackCount);
	ClearHoverItem();
	ShowCursor();
	
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(GridSlots[GridIndex]->GetInventoryItem().Get(), FragmentTags::GridFragment);
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
	HighlightSlots(GridIndex, Dimensions);
}

void UInv_InventoryGrid::FillInStack(const int32 GridIndex, const int32 FillAmount, const int32 Remainder)
{
	UInv_GridSlot* GridSlot = GridSlots[GridIndex];
	UInv_SlottedItem* ClickedSlottedItem = SlottedItems.FindChecked(GridIndex);
	int32 NewStackCount = GridSlot->GetStackCount() + FillAmount;
	GridSlot->SetStackCount(NewStackCount);
	ClickedSlottedItem->SetStackCount(NewStackCount);
	HoverItem->SetStackCount(Remainder);
}

void UInv_InventoryGrid::PutBackHoverItem()
{
	if (!IsValid(HoverItem)) return;
	// Hover Item doesn't have ItemManifest which HasRoomForItem requires to calculate stack count from, that's why we 
	// create the stack count override
	FInv_SlotAvailabilityResult Result = HasRoomForItem(HoverItem->GetInventoryItem(), HoverItem->GetStackCount());
	Result.Item = HoverItem->GetInventoryItem();
	
	AddStacks(Result);
	ClearHoverItem();
}


void UInv_InventoryGrid::CreateHoverItemWidget(const int32 PrevGridIndex, UInv_InventoryItem* ClickedInventoryItem)
{
	AssignHoverItem(ClickedInventoryItem);
	
	HoverItem->SetPreviousGridIndex(PrevGridIndex);
	HoverItem->SetStackCount(HoverItem->IsStackable() ? GridSlots[PrevGridIndex]->GetStackCount() : 0);
}

void UInv_InventoryGrid::AssignHoverItem(UInv_InventoryItem* ClickedInventoryItem)
{
	// Pickup Item - Assign the hover item and remove the slotted item from the grid.
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(ClickedInventoryItem, FragmentTags::GridFragment);
	const FInv_ImageFragment* ImageFragment = GetFragment<FInv_ImageFragment>(ClickedInventoryItem, FragmentTags::IconFragment);
	if (!GridFragment || !ImageFragment) return;
	if (!IsValid(HoverItem))
	{
		HoverItem = CreateWidget<UInv_HoverItem>(GetOwningPlayer(), HoverItemClass);
	}
	FSlateBrush IconBrush;
	IconBrush.SetResourceObject(ImageFragment->GetIcon());
	IconBrush.DrawAs = ESlateBrushDrawType::Image;
	IconBrush.ImageSize = GetDrawSize(GridFragment) * UWidgetLayoutLibrary::GetViewportScale(this); // Scale draw size in case the viewport is in a bad scale
	HoverItem->SetImageBrush(IconBrush);
	HoverItem->SetGridDimensions(GridFragment->GetGridSize());
	HoverItem->SetInventoryItem(ClickedInventoryItem);
	HoverItem->SetIsStackable(ClickedInventoryItem->IsStackable());
	
	GetOwningPlayer()->SetMouseCursorWidget(EMouseCursor::Default, HoverItem);
}

void UInv_InventoryGrid::OnHide()
{
	PutBackHoverItem();
}

void UInv_InventoryGrid::RemoveItemFromGrid(const int32 GridIndex, const UInv_InventoryItem* ClickedInventoryItem)
{
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(ClickedInventoryItem, FragmentTags::GridFragment);
	if (!GridFragment) return;
	UInv_InventoryStatics::ForEach2D(GridSlots, GridIndex, GridFragment->GetGridSize(), Columns, [&](UInv_GridSlot* GridSlot)
	{
		GridSlot->SetInventoryItem(nullptr);
		GridSlot->SetUpperLeftIndex(INDEX_NONE);
		GridSlot->SetUnoccupiedTexture();
		GridSlot->SetAvailable(true);
		GridSlot->SetStackCount(0);
	});
	
	if (SlottedItems.Contains(GridIndex))
	{
		TObjectPtr<UInv_SlottedItem> FoundSlottedItem;
		SlottedItems.RemoveAndCopyValue(GridIndex, FoundSlottedItem);
		FoundSlottedItem->RemoveFromParent();
	}
}

void UInv_InventoryGrid::AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment,
                                                UInv_SlottedItem* SlottedItem) const
{
	CanvasPanel->AddChild(SlottedItem);
	UCanvasPanelSlot* CanvasSlot = UWidgetLayoutLibrary::SlotAsCanvasSlot(SlottedItem);
	CanvasSlot->SetSize(GetDrawSize(GridFragment));
	const FVector2D DrawPosition = UInv_WidgetUtils::GetPositionFromIndex(Index, Columns) * TileSize;
	const FVector2D DrawPositionWithPadding = DrawPosition + FVector2D(GridFragment->GetGridPadding());
	CanvasSlot->SetPosition(DrawPositionWithPadding);
}

void UInv_InventoryGrid::UpdateGridSlots(UInv_InventoryItem* NewInventoryItem, const int32 Index, const bool bStackable, const int32 StackAmount)
{
	check(GridSlots.IsValidIndex(Index));
	
	if (bStackable)
	{
		GridSlots[Index]->SetStackCount(StackAmount);
	}
	
	
	const FInv_GridFragment* GridFragment = GetFragment<FInv_GridFragment>(NewInventoryItem, FragmentTags::GridFragment);
	const FIntPoint Dimensions = GridFragment ? GridFragment->GetGridSize() : FIntPoint(1, 1);
	
	UInv_InventoryStatics::ForEach2D(GridSlots, Index, Dimensions, Columns, [&](UInv_GridSlot* GridSlot)
	{
		GridSlot->SetInventoryItem(NewInventoryItem);
		GridSlot->SetUpperLeftIndex(Index);
		GridSlot->SetOccupiedTexture();
		GridSlot->SetAvailable(false);
	});
}


bool UInv_InventoryGrid::MatchesCategory(const UInv_InventoryItem* InventoryItem) const
{
	return InventoryItem->GetItemManifest().GetItemCategory() == ItemCategory;
}




FVector2D UInv_InventoryGrid::GetDrawSize(const FInv_GridFragment* GridFragment) const
{
	// const float IconTileWidth = TileSize - GridFragment->GetGridPadding() * 2;
	const FVector2D IconSize = GridFragment->GetGridSize() * TileSize - GridFragment->GetGridPadding() * 2;
	return IconSize;
}

void UInv_InventoryGrid::SetSlottedItemImage(const UInv_SlottedItem* SlottedItem, const FInv_GridFragment* GridFragment,
                                             const FInv_ImageFragment* ImageFragment) const
{
	FSlateBrush Brush;
	Brush.SetResourceObject(ImageFragment->GetIcon());
	Brush.DrawAs = ESlateBrushDrawType::Image;
	Brush.ImageSize = GetDrawSize(GridFragment);
	SlottedItem->SetImageBrush(Brush);
}
