// Fill out your copyright notice in the Description page of Project Settings.


#include "Widgets/Utils/Inv_WidgetUtils.h"

#include "Blueprint/SlateBlueprintLibrary.h"
#include "Components/Widget.h"

int32 UInv_WidgetUtils::GetIndexFromPosition(const FIntPoint& Position, const int32 Columns)
{
	return Position.X + (Position.Y * Columns);
}

FIntPoint UInv_WidgetUtils::GetPositionFromIndex(const int32 Index, const int32 Columns)
{
	return FIntPoint(Index % Columns, Index / Columns);
}

bool UInv_WidgetUtils::IsWithinWidgetBounds(const FVector2D& Position, const FVector2D& Size, const FVector2D& Location)
{
	
	return Location.X >= Position.X && Location.Y >= Position.Y && Location.X <= Position.X + Size.X && Location.Y <= Position.Y + Size.Y;
}

FVector2D UInv_WidgetUtils::GetClampedWidgetPosition(const FVector2D& Boundary, const FVector2D& WidgetSize,
	const FVector2D& MousePosition)
{
	FVector2D ClampedPosition = MousePosition;
	// Adjust horizontal pos to ensure that the widget stays within the boundary
	if (MousePosition.X + WidgetSize.X > Boundary.X) ClampedPosition.X = Boundary.X - WidgetSize.X;
	if (MousePosition.X < 0) ClampedPosition.X = 0;
	if (MousePosition.Y + WidgetSize.Y > Boundary.Y) ClampedPosition.Y = Boundary.Y - WidgetSize.Y;
	if (MousePosition.Y < 0) ClampedPosition.Y = 0;
	return ClampedPosition;
}

FVector2D UInv_WidgetUtils::GetWidgetPosition(UWidget* Widget)
{
	const FGeometry Geometry = Widget->GetCachedGeometry();
	FVector2D PixelPosition;
	FVector2D ViewportPosition;
	USlateBlueprintLibrary::LocalToViewport(Widget, Geometry, 
		USlateBlueprintLibrary::GetLocalTopLeft(Geometry), 
		PixelPosition, ViewportPosition); // Getting the top left position of widget in viewport space 
	return ViewportPosition;
}

FVector2D UInv_WidgetUtils::GetWidgetSize(UWidget* Widget)
{
	const FGeometry Geometry = Widget->GetCachedGeometry();
	return Geometry.GetLocalSize();
}
