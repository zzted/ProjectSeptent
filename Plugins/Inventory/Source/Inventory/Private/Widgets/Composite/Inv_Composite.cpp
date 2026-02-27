// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Composite/Inv_Composite.h"

#include "Blueprint/WidgetTree.h"

void UInv_Composite::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	// Add widgets in the widget tree to the children if they are of composite class 
	WidgetTree->ForEachWidget([this](UWidget* Widget)
	{
		UInv_CompositeBase* Composite = Cast<UInv_CompositeBase>(Widget);
		if (IsValid(Composite))
		{
			Children.Add(Composite);
			Composite->Collapse();
		}
	});
}

void UInv_Composite::ApplyFunction(FuncType Function)
{
	for (UInv_CompositeBase* Child : Children)
	{
		Child->ApplyFunction(Function);
	}
}

void UInv_Composite::Collapse()
{
	for (UInv_CompositeBase* Child : Children)
	{
		Child->Collapse();
	}
}
