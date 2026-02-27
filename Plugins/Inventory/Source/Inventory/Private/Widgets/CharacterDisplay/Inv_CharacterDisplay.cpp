// Fill out your copyright notice in the Description page of Project Settings.

#include "Widgets/CharacterDisplay/Inv_CharacterDisplay.h"
#include "Blueprint/WidgetLayoutLibrary.h"
#include "EquipmentManagement/ProxyMesh/Inv_ProxyMeshActor.h"
#include "Kismet/GameplayStatics.h"

void UInv_CharacterDisplay::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(this, AInv_ProxyMeshActor::StaticClass(), Actors);
	
	if (!Actors.IsValidIndex(0)) return;
	AInv_ProxyMeshActor* ProxyMeshActor = Cast<AInv_ProxyMeshActor>(Actors[0]);
	if (!IsValid(ProxyMeshActor)) return;
	
	ProxyMesh = ProxyMeshActor->GetProxyMesh();
}

void UInv_CharacterDisplay::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);
	if (!bIsDragging) return;
	
	LastPosition = CurrentPosition;
	CurrentPosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
	
	const float HorizontalDelta = LastPosition.X - CurrentPosition.X;
	if (!ProxyMesh.IsValid()) return;
	ProxyMesh->AddRelativeRotation(FRotator(0.f, HorizontalDelta, 0.f));
}

FReply UInv_CharacterDisplay::NativeOnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	CurrentPosition = UWidgetLayoutLibrary::GetMousePositionOnViewport(GetOwningPlayer());
	LastPosition = CurrentPosition;
	
	bIsDragging = true;
	return FReply::Handled();
}

FReply UInv_CharacterDisplay::NativeOnMouseButtonUp(const FGeometry& InGeometry, const FPointerEvent& InMouseEvent)
{
	bIsDragging = false;
	return Super::NativeOnMouseButtonUp(InGeometry, InMouseEvent);
}

void UInv_CharacterDisplay::NativeOnMouseLeave(const FPointerEvent& MouseEvent)
{
	Super::NativeOnMouseLeave(MouseEvent);
	bIsDragging = false;
}
