// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Inv_Leaf.h"
#include "Inv_Leaf_LabeledValue.generated.h"

class UTextBlock;
/**
 * 
 */
UCLASS()
class INVENTORY_API UInv_Leaf_LabeledValue : public UInv_Leaf
{
	GENERATED_BODY()
public:
	void SetLabel(const FText& Label, bool bCollapse) const;
	void SetValue(const FText& Value, bool bCollapse) const;
	virtual void NativePreConstruct() override;
	
private:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Label;
	
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UTextBlock> Text_Value;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 FontSize_Label = 12;
	
	UPROPERTY(EditAnywhere, Category="Inventory")
	int32 FontSize_Value = 18;
};
