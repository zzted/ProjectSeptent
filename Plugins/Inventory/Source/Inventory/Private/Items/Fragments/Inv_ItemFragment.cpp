#include "Items/Fragments/Inv_ItemFragment.h"

#include "EquipmentManagement/EquipActor/Inv_EquipActor.h"
#include "Widgets/Composite/Inv_CompositeBase.h"
#include "Widgets/Composite/Inv_Leaf_Image.h"
#include "Widgets/Composite/Inv_Leaf_LabeledValue.h"
#include "Widgets/Composite/Inv_Leaf_Text.h"

void FInv_InventoryItemFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	if (!MatchesWidgetTag(Composite)) return;
	Composite->Expand();
}

bool FInv_InventoryItemFragment::MatchesWidgetTag(const UInv_CompositeBase* Composite) const
{
	return Composite->GetFragmentTag().MatchesTagExact(GetFragmentTag());
}

void FInv_ImageFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragment::Assimilate(Composite);
	if (!MatchesWidgetTag(Composite)) return;
	
	UInv_Leaf_Image* Image = Cast<UInv_Leaf_Image>(Composite);
	if (!IsValid(Image)) return;
	Image->SetImage(Icon);
	Image->SetBoxSize(IconDimensions);
	Image->SetImageSize(IconDimensions);
}

void FInv_TextFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragment::Assimilate(Composite);
	if (!MatchesWidgetTag(Composite)) return;

	const UInv_Leaf_Text* LeafText = Cast<UInv_Leaf_Text>(Composite);
	if (!IsValid(LeafText)) return;
	LeafText->SetText(FragmentText);
}

void FInv_LabeledNumberFragment::Manifest()
{
	FInv_InventoryItemFragment::Manifest();
	if (bRandomizeOnManifest)
	{
		Value = FMath::RandRange(Min, Max);
	}
	bRandomizeOnManifest = false;
}

void FInv_LabeledNumberFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragment::Assimilate(Composite);
	if (!MatchesWidgetTag(Composite)) return;
	
	UInv_Leaf_LabeledValue* LabeledValue = Cast<UInv_Leaf_LabeledValue>(Composite);
	if (!IsValid(LabeledValue)) return;
	LabeledValue->SetLabel(Text_Label, bCollapseLabel);
	
	FNumberFormattingOptions NumberFormattingOptions;
	NumberFormattingOptions.MinimumFractionalDigits = MinFractionalDigits;
	NumberFormattingOptions.MaximumFractionalDigits = MaxFractionalDigits;
	LabeledValue->SetValue(FText::AsNumber(Value, &NumberFormattingOptions), bCollapseValue);
}

void FInv_ConsumableFragment::OnConsume(APlayerController* PC)
{
	for (TInstancedStruct<FInv_ConsumeModifier>& Modifier : ConsumeModifiers)
	{
		FInv_ConsumeModifier& ModRef = Modifier.GetMutable();
		ModRef.OnConsume(PC);
	}
}

void FInv_ConsumableFragment::Manifest()
{
	FInv_InventoryItemFragment::Manifest();
	for (TInstancedStruct<FInv_ConsumeModifier>& Modifier : ConsumeModifiers)
	{
		FInv_ConsumeModifier& ModRef = Modifier.GetMutable();
		ModRef.Manifest();
	}
}

void FInv_ConsumableFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragment::Assimilate(Composite);
	for (const TInstancedStruct<FInv_ConsumeModifier>& Modifier : ConsumeModifiers)
	{
		const FInv_ConsumeModifier& ModRef = Modifier.Get();
		ModRef.Assimilate(Composite);
	}
}

void FInv_HealthPotionFragment::OnConsume(APlayerController* PC)
{
	// Get stats component from the PC or the PC->GetPawn()
	// or get ASC and apply a Gameplay Effect
	// or call and interface function for Healing()
	
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, 
		FString::Printf(TEXT("Healed %f HP"), Value));
}

void FInv_ManaPotionFragment::OnConsume(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Blue, 
		FString::Printf(TEXT("Restored %f MANA"), Value));
}

void FInv_StrengthModifier::OnEquip(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, 
		FString::Printf(TEXT("Strength increased by %f."), Value));
}

void FInv_StrengthModifier::OnUnequip(APlayerController* PC)
{
	GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, 
		FString::Printf(TEXT("Strength decreased by %f."), Value));
}

void FInv_EquipmentFragment::OnEquip(APlayerController* PC)
{
	if (bEquipped) return;
	bEquipped = true;
	for (TInstancedStruct<FInv_EquipModifier>& Modifier : EquipModifiers)
	{
		FInv_EquipModifier& ModRef = Modifier.GetMutable();
		ModRef.OnEquip(PC);
	}
}

void FInv_EquipmentFragment::OnUnequip(APlayerController* PC)
{
	if (!bEquipped) return;
	bEquipped = false;
	for (TInstancedStruct<FInv_EquipModifier>& Modifier : EquipModifiers)
	{
		FInv_EquipModifier& ModRef = Modifier.GetMutable();
		ModRef.OnUnequip(PC);
	}
}

void FInv_EquipmentFragment::Assimilate(UInv_CompositeBase* Composite) const
{
	FInv_InventoryItemFragment::Assimilate(Composite);
	for (const TInstancedStruct<FInv_EquipModifier>& Modifier : EquipModifiers)
	{
		const FInv_EquipModifier& ModRef = Modifier.Get();
		ModRef.Assimilate(Composite);
	}
}

void FInv_EquipmentFragment::Manifest()
{
	FInv_InventoryItemFragment::Manifest();
	for (TInstancedStruct<FInv_EquipModifier>& Modifier : EquipModifiers)
	{
		FInv_EquipModifier& ModRef = Modifier.GetMutable();
		ModRef.Manifest();
	}
}

AInv_EquipActor* FInv_EquipmentFragment::SpawnAttachedActor(USkeletalMeshComponent* AttachMesh) const
{
	if (!IsValid(EquipActorClass) || !IsValid(AttachMesh)) return nullptr;
	AInv_EquipActor* SpawnedActor = AttachMesh->GetWorld()->SpawnActor<AInv_EquipActor>(EquipActorClass);
	SpawnedActor->AttachToComponent(AttachMesh, FAttachmentTransformRules::SnapToTargetNotIncludingScale, SocketAttachPoint);
	return SpawnedActor;
}

void FInv_EquipmentFragment::DestroyAttachedActor()
{
	if (EquippedActor.IsValid())
	{
		EquippedActor->Destroy();
	}
}

void FInv_EquipmentFragment::SetEquippedActor(AInv_EquipActor* EquipActor)
{
	EquippedActor = EquipActor;
}
