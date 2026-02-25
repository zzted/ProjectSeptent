// ZZ

#pragma once

#include "CoreMinimal.h"
#include "MVVMViewModelBase.h"
#include "MVVM_LoadScreen.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FSlotSelected);

class UMVVM_LoadSlot;
/**
 * 
 */
UCLASS()
class SEPTENT_API UMVVM_LoadScreen : public UMVVMViewModelBase
{
	GENERATED_BODY()

public:
	void InitializeLoadSlotVMs();

	UPROPERTY(BlueprintAssignable)
	FSlotSelected OnSlotSelected;
	
	UPROPERTY(EditDefaultsOnly)
	TSubclassOf<UMVVM_LoadSlot> LoadSlotViewModelClass;

	UFUNCTION(BlueprintPure)
	UMVVM_LoadSlot* GetLoadSlotViewModelByIndex(int32 SlotIndex) const;

	UFUNCTION(BlueprintCallable)
	void NewSlotButtonPressed(int32 SlotIndex, const FString& EnteredName);

	UFUNCTION(BlueprintCallable)
	void NewGameButtonPressed(int32 SlotIndex);

	UFUNCTION(BlueprintCallable)
	void SelectSlotButtonPressed(int32 SlotIndex);
	
	UFUNCTION(BlueprintCallable)
	void DeleteButtonPressed();

	UFUNCTION(BlueprintCallable)
	void PlayButtonPressed();

	void LoadData();

	void SetNumLoadSlots(int32 InNumLoadSlots);
	int32 GetNumLoadSlots() const { return NumLoadSlots; }

private:
	UPROPERTY()
	TMap<int32, UMVVM_LoadSlot*> LoadSlotVMs;

	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlotVM_0;

	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlotVM_1;

	UPROPERTY()
	TObjectPtr<UMVVM_LoadSlot> LoadSlotVM_2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, FieldNotify, Setter, Getter, meta = (AllowPrivateAccess="true"))
	int32 NumLoadSlots;

	UPROPERTY()
	UMVVM_LoadSlot* SelectedSlot;
	
};
