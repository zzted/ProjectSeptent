#pragma once

#include "CoreMinimal.h"
#include "GameplayTagContainer.h"
#include "StructUtils/InstancedStruct.h"
#include "Inv_ItemFragment.generated.h"

class AInv_EquipActor;
class UInv_CompositeBase;
/**
 * Why we use FInstancedStruct instead of UCLASS for Item Fragments:
 *
 * FInstancedStruct is designed for data-oriented, value-type polymorphism,
 * whereas UCLASS is designed for UObject-based objects with complex lifecycles.
 *
 * Key reasons for using FInstancedStruct:
 * 
 * 1. Value Semantics and Copying:
 *    - InstancedStruct supports copying by value, which is essential for
 *      items/fragments that are duplicated frequently (e.g., inventory items,
 *      undo/redo operations, serialization).
 *    - UCLASS instances are always referenced by pointer, so copying a UObject
 *      requires duplication logic and can lead to shared-state bugs.
 *
 * 2. Low Overhead and Performance:
 *    - InstancedStruct does not participate in the Garbage Collector, so
 *      allocation and destruction are lightweight and predictable.
 *    - UCLASS objects require heap allocation, GC tracking, and reference management,
 *      which adds unnecessary overhead for high-frequency small data types.
 *
 * 3. Safe and Flexible Lifecycle Management:
 *    - InstancedStruct works with UScriptStruct metadata to safely construct,
 *      copy, move, and destruct polymorphic structs without relying on UObject lifecycle.
 *    - Virtual destructors in base structs ensure proper cleanup of derived fragments.
 *
 * 4. Editor, Networking, and Serialization Friendly:
 *    - InstancedStruct integrates seamlessly with FastArray serialization,
 *      replication, and editor copy/paste operations for data blocks.
 *    - UCLASS replication and duplication semantics are more heavyweight and
 *      prone to pointer aliasing issues.
 *
 * Summary:
 * - Use FInstancedStruct for lightweight, composable, and polymorphic data fragments.
 * - Use UCLASS only for objects that need world context, behavior, or complex lifecycle management.
 */
USTRUCT(BlueprintType)
struct FInv_ItemFragment
{
	GENERATED_BODY()
	FInv_ItemFragment() {}
	FInv_ItemFragment(const FInv_ItemFragment&) = default;
	FInv_ItemFragment& operator=(const FInv_ItemFragment&) = default;
	FInv_ItemFragment(FInv_ItemFragment&&) = default;
	FInv_ItemFragment& operator=(FInv_ItemFragment&&) = default;
	virtual ~FInv_ItemFragment() {} // virtual destructor is required here
	
	FGameplayTag GetFragmentTag() const { return FragmentTag; }
	void SetFragmentTag(FGameplayTag Tag) { FragmentTag = Tag; }
	virtual void Manifest() {}
	
private:
	UPROPERTY(EditAnywhere, Category="Inventory", meta=(Categories="FragmentTags"))
	FGameplayTag FragmentTag = FGameplayTag::EmptyTag;
};

/*
 * Item fragment specifically for assimilation into a widget
 */
USTRUCT()
struct FInv_InventoryItemFragment : public FInv_ItemFragment
{
	GENERATED_BODY()
	
	virtual void Assimilate(UInv_CompositeBase* Composite) const;
protected:
	bool MatchesWidgetTag(const UInv_CompositeBase* Composite) const;
};

USTRUCT(BlueprintType)
struct FInv_GridFragment : public FInv_ItemFragment
{
	GENERATED_BODY()
	
	FIntPoint GetGridSize() const { return GridSize; }
	void SetGridSize(FIntPoint Size) { GridSize = Size; }
	float GetGridPadding() const { return GridPadding; }
	void SetGridPadding(float Padding) { GridPadding = Padding; }
	
private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	FIntPoint GridSize = {1, 1};
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	float GridPadding = 0.0f;
};

USTRUCT(BlueprintType)
struct FInv_ImageFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()
	
	UTexture2D* GetIcon() const { return Icon; }
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;
	
private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	TObjectPtr<UTexture2D> Icon = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	FVector2D IconDimensions = FVector2D(64.f, 64.f);
};

USTRUCT(BlueprintType)
struct FInv_TextFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()
	
	FText GetText() const { return FragmentText; }
	void SetText(const FText& Text) { FragmentText = Text; }
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;

private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	FText FragmentText;
};

USTRUCT(BlueprintType)
struct FInv_LabeledNumberFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()

	virtual void Manifest() override;
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;
	
	// When manifesting for the first time, this fragment will randomize. Once Equipped and dropped item should retain
	// the same value, so randomization should not occur.
	bool bRandomizeOnManifest = true;

protected:
	UPROPERTY(EditAnywhere, Category="Inventory")
	FText Text_Label = {};
	
	UPROPERTY(VisibleAnywhere, Category="Inventory")
	float Value = 0.f;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	float Min = 0;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	float Max = 0;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	bool bCollapseLabel = false;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	bool bCollapseValue = false;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 MinFractionalDigits = 1;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 MaxFractionalDigits = 1;
};

USTRUCT(BlueprintType)
struct FInv_StackableFragment : public FInv_ItemFragment
{
	GENERATED_BODY()
	
	int32 GetMaxStackSize() const { return MaxStackSize; }
	int32 GetStackCount() const { return StackCount; }
	void SetStackCount(const int32 Count) { StackCount = Count; }
	
private:
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 MaxStackSize = 1;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 StackCount = 1;
};

/*
 * Consume Fragments
 */

USTRUCT(BlueprintType)
struct FInv_ConsumeModifier : public FInv_LabeledNumberFragment
{
	GENERATED_BODY()
	
	virtual void OnConsume(APlayerController* PC) {}
	
};

USTRUCT(BlueprintType)
struct FInv_ConsumableFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()
	
	virtual void OnConsume(APlayerController* PC);
	
	virtual void Manifest() override;
	
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;
	
private:
	UPROPERTY(EditAnywhere, Category="Inventory", meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FInv_ConsumeModifier>> ConsumeModifiers;
};

USTRUCT(BlueprintType)
struct FInv_HealthPotionFragment : public FInv_ConsumeModifier
{
	GENERATED_BODY()
	
	virtual void OnConsume(APlayerController* PC) override;
};

USTRUCT(BlueprintType)
struct FInv_ManaPotionFragment : public FInv_ConsumeModifier
{
	GENERATED_BODY()
	
	virtual void OnConsume(APlayerController* PC) override;
};

/*
 * Equipments
 */

USTRUCT(BlueprintType)
struct FInv_EquipModifier : public FInv_LabeledNumberFragment
{
	GENERATED_BODY()
	
	virtual void OnEquip(APlayerController* PC) {}
	virtual void OnUnequip(APlayerController* PC) {}
};

USTRUCT(BlueprintType)
struct FInv_StrengthModifier : public FInv_EquipModifier
{
	GENERATED_BODY()
	
	virtual void OnEquip(APlayerController* PC) override;
	virtual void OnUnequip(APlayerController* PC) override;
};

USTRUCT(BlueprintType)
struct FInv_EquipmentFragment : public FInv_InventoryItemFragment
{
	GENERATED_BODY()
	
	bool bEquipped = false;
	void OnEquip(APlayerController* PC);
	void OnUnequip(APlayerController* PC);
	virtual void Assimilate(UInv_CompositeBase* Composite) const override;
	virtual void Manifest() override;
	
	AInv_EquipActor* SpawnAttachedActor(USkeletalMeshComponent* AttachMesh) const;
	void DestroyAttachedActor();
	FGameplayTag GetEquipmentTypeTag() const { return EquipmentTypeTag; }
	void SetEquippedActor(AInv_EquipActor* EquipActor);
	
private:
	UPROPERTY(EditAnywhere, Category="Inventory", meta=(ExcludeBaseStruct))
	TArray<TInstancedStruct<FInv_EquipModifier>> EquipModifiers;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	TSubclassOf<AInv_EquipActor> EquipActorClass = nullptr;
	
	TWeakObjectPtr<AInv_EquipActor> EquippedActor = nullptr;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	FName SocketAttachPoint = NAME_None;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	FGameplayTag EquipmentTypeTag = FGameplayTag::EmptyTag;
};
