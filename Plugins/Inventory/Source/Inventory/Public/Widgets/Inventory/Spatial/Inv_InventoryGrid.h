// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/Inv_GridTypes.h"
#include "Inv_InventoryGrid.generated.h"

class UInv_ItemPopUp;
enum class EInv_GridSlotState : uint8;
class UInv_HoverItem;
struct FGameplayTag;
struct FInv_ImageFragment;
struct FInv_GridFragment;
struct FInv_ItemManifest;
class UInv_ItemComponent;
class UInv_InventoryComponent;
class UCanvasPanel;
class UInv_GridSlot;
class UInv_SlottedItem;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryGrid : public UUserWidget
{
	GENERATED_BODY()
public:
	virtual void NativeOnInitialized() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override; // Tick function for the widget type
	
	EInv_ItemCategory GetItemCategory() const {return ItemCategory;}
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_ItemComponent* ItemComponent);
	
	void ShowCursor();
	void HideCursor();
	void SetOwningCanvasPanel(UCanvasPanel* OwningCanvas);
	void DropItem();
	bool HasHoverItem() const ;
	UInv_HoverItem* GetHoverItem();
	void ClearHoverItem();
	float GetTileSize() const { return TileSize; }
	void AssignHoverItem(UInv_InventoryItem* ClickedInventoryItem);
	void OnHide();
	
	UFUNCTION()
	void AddItem(UInv_InventoryItem* InventoryItem);
	
	UFUNCTION()
	void OnSlottedItemClicked(int32 GridIndex, const FPointerEvent& MouseEvent);

private:
	
	TWeakObjectPtr<UInv_InventoryComponent> InventoryComponent;
	TWeakObjectPtr<UCanvasPanel> OwningCanvasPanel;
	
	void ConstructGrid();
	bool MatchesCategory(const UInv_InventoryItem* InventoryItem) const;
	
	/* Callbacks */
	UFUNCTION()
	void OnGridSlotClicked(int32 GridIndex, const FPointerEvent& MouseEvent);
	
	UFUNCTION()
	void OnGridSlotHovered(int32 GridIndex, const FPointerEvent& MouseEvent);
	
	UFUNCTION()
	void OnGridSlotUnhovered(int32 GridIndex, const FPointerEvent& MouseEvent);
	
	UFUNCTION()
	void OnPopUpMenuSplit(int32 SplitAmount, int32 GridIndex);
	
	UFUNCTION()
	void OnPopUpMenuDrop(int32 GridIndex);
	
	UFUNCTION()
	void OnPopUpMenuConsume(int32 GridIndex);
	
	UFUNCTION()
	void OnInventoryMenuToggled(bool bIsOpen);
	
	/* Add Stack to Existing Slot */
	UFUNCTION()
	void AddStacks(const FInv_SlotAvailabilityResult& Result);
	
	/* Create Slotted Item at Index */
	void AddItemToIndices(const FInv_SlotAvailabilityResult& Result, UInv_InventoryItem* NewInventoryItem);
	void AddItemAtIndex(UInv_InventoryItem* NewInventoryItem, int32 Index, const bool bStackable, const int32 StackAmount);
	UInv_SlottedItem* CreateSlottedItem(UInv_InventoryItem* NewInventoryItem, int32 Index,
	                                                 const FInv_GridFragment* GridFragment,
	                                                 const FInv_ImageFragment* ImageFragment, 
	                                                 const bool bStackable, 
	                                                 const int32 StackAmount) const;
	void SetSlottedItemImage(const UInv_SlottedItem* SlottedItem, const FInv_GridFragment* GridFragment, const FInv_ImageFragment* ImageFragment) const;
	FVector2D GetDrawSize(const FInv_GridFragment* GridFragment) const;
	void AddSlottedItemToCanvas(const int32 Index, const FInv_GridFragment* GridFragment, UInv_SlottedItem* SlottedItem) const;
	void UpdateGridSlots(UInv_InventoryItem* NewInventoryItem, const int32 Index, bool bStackable, const int32 StackAmount);
	
	/* Check SlotAvailability */
	FInv_SlotAvailabilityResult HasRoomForItem(const UInv_InventoryItem* InventoryItem, const int32 StackAmountOverride = -1);
	FInv_SlotAvailabilityResult HasRoomForItem(const FInv_ItemManifest& Manifest, const int32 StackAmountOverride = -1);
	bool HasRoomAtIndex(const UInv_GridSlot* GridSlot, const FIntPoint& Dimensions, const TSet<int32>& OccupiedIndices,
	                    TSet<int32>& TentativelyOccupiedIndices, const FGameplayTag& ItemType, const int32 MaxStackSize);
	bool CheckSlotConstrains(const UInv_GridSlot* GridSlot, const UInv_GridSlot* SubGridSlot, const TSet<int32>& OccupiedIndices, const FGameplayTag& ItemType, const int32 MaxStackSize);
	bool IsInGridBounds(const int32 StartIndex, const FIntPoint& ItemDimensions) const;
	int32 DetermineFillAmountForSlot(const bool bStackable, const int32 MaxStackSize, const int32 AmountToFill, const UInv_GridSlot* GridSlot);
	
	/* Hover Item */
	void CreateHoverItemWidget(int32 PrevGridIndex, UInv_InventoryItem* ClickedInventoryItem);
	void RemoveItemFromGrid(int32 GridIndex, const UInv_InventoryItem* ClickedInventoryItem);
	void UpdateTileParameters(const FVector2D& CanvasPosition, const FVector2D& MousePosition);
	EInv_TileQuadrant CalculateTileQuadrant(const FVector2D& CanvasPosition, const FVector2D& MousePosition);
	void OnTileParametersUpdate(const FInv_TileParameters& NewTileParameters);
	FIntPoint CalculateStartingCoordinates(const FIntPoint& Coordinate, const FIntPoint& Dimensions, const EInv_TileQuadrant Quadrant);
	FInv_SpaceQueryResult CheckHoverPosition(const FIntPoint& Position, const FIntPoint& Dimensions);
	
	void HighlightSlots(const int32 Index, const FIntPoint& Dimensions);
	void UnhighlightSlots(const int32 Index, const FIntPoint& Dimensions);
	void ChangeHoverHighlightType(const int32 Index, const FIntPoint& Dimensions, EInv_GridSlotState GridSlotState);
	
	/* Put Down Hover Item*/

	bool IsSameStackable(const UInv_InventoryItem* ClickedInventoryItem) const;
	void SwapWithHoverItem(int32 GridIndex, UInv_InventoryItem* ClickedInventoryItem);
	void SwapStackCounts(const int32 GridIndex, const int32 ClickedStackCount, const int32 HoverStackCount);
	void ConsumeHoverItemStack(const int32 GridIndex, const int32 ClickedStackCount, const int32 HoverStackCount);
	void FillInStack(const int32 GridIndex, const int32 FillAmount, const int32 Remainder);
	void PutBackHoverItem();
	
	/* Cursor */
	UUserWidget* GetVisibleCursorWidget();
	UUserWidget* GetHiddenCursorWidget();
	
	/* PopUp Menu */
	void CreateItemPopUp(const int32 GridIndex);
	
	
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UUserWidget> ItemPopUpClass;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UUserWidget> VisibleCursorWidgetClass;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UUserWidget> HiddenCursorWidgetClass;
	
	UPROPERTY()
	TObjectPtr<UInv_ItemPopUp> ItemPopUp;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	FVector2D ItemPopUpOffset;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> VisibleCursorWidget;
	
	UPROPERTY()
	TObjectPtr<UUserWidget> HiddenCursorWidget;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"), Category="Inventory")
	EInv_ItemCategory ItemCategory;
	
	UPROPERTY()
	TArray<TObjectPtr<UInv_GridSlot>> GridSlots;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInv_GridSlot> GridSlotClass;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInv_SlottedItem> SlottedItemClass;
	
	UPROPERTY()
	TMap<int32 /*Grid Index*/, TObjectPtr<UInv_SlottedItem>> SlottedItems;
	
	UPROPERTY(meta = (BindWidget))
	TObjectPtr<UCanvasPanel> CanvasPanel;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 Rows;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 Columns;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	float TileSize;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<UInv_HoverItem> HoverItemClass;
	
	UPROPERTY()
	TObjectPtr<UInv_HoverItem> HoverItem;
	
	FInv_TileParameters TileParameters;
	FInv_TileParameters LastTileParameters;
	
	// Index where an item would be placed if we click on the grid at a valid location
	int32 ItemDropIndex = INDEX_NONE;
	
	FInv_SpaceQueryResult CurrentSpaceQueryResult;
	
	bool bMouseWithinCanvas;
	bool bLastMouseWithinCanvas;
	
	int32 LastHighlightedIndex = INDEX_NONE;
	FIntPoint LastHighlightedDimensions;
};
