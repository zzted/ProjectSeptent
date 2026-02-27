// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/Inv_GridTypes.h"
#include "Widgets/Utils/Inv_WidgetUtils.h"
#include "Inv_InventoryStatics.generated.h"

class UInv_InventoryBase;
class UInv_HoverItem;
class UInv_ItemComponent;
class UInv_InventoryComponent;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_InventoryStatics : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
public:
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static UInv_InventoryComponent* GetInventoryComponent(const APlayerController* PlayerController);
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static EInv_ItemCategory GetItemCategoryFromItemComp(const UInv_ItemComponent* ItemComponent);
	
	template<typename T, typename FuncT>
	static void ForEach2D(TArray<T>& Array, int32 Index, const FIntPoint& Range2D, int32 GridColumns, const FuncT& Function);
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static void ItemHovered(APlayerController* PlayerController, UInv_InventoryItem* Item);
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static void ItemUnhovered(APlayerController* PlayerController);
	
	UFUNCTION(BlueprintCallable, Category="Inventory")
	static UInv_HoverItem* GetHoverItem(APlayerController* PlayerController);
	
	static UInv_InventoryBase* GetInventoryMenu(APlayerController* PlayerController);
};

template <typename T, typename FuncT>
void UInv_InventoryStatics::ForEach2D(TArray<T>& Array, const int32 Index, const FIntPoint& Range2D, const int32 GridColumns,
	const FuncT& Function)
{
	for (int32 i = 0; i < Range2D.Y; ++i)
	{
		for (int32 j = 0; j < Range2D.X; ++j)
		{
			const FIntPoint Coordinates = UInv_WidgetUtils::GetPositionFromIndex(Index, GridColumns) + FIntPoint(j, i); // starting index 
			const int32 TileIndex = UInv_WidgetUtils::GetIndexFromPosition(Coordinates, GridColumns); // actual index of the tile
			if (Array.IsValidIndex(TileIndex))
			{
				Function(Array[TileIndex]);
			}
		}
	}
}
