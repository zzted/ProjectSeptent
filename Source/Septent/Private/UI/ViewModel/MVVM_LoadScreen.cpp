// ZZ


#include "UI/ViewModel/MVVM_LoadScreen.h"

#include "Game/SeptentGameInstance.h"
#include "Game/SeptentGameModeBase.h"
#include "Kismet/GameplayStatics.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void UMVVM_LoadScreen::InitializeLoadSlotVMs()
{
	LoadSlotVM_0 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlotVM_1 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlotVM_2 = NewObject<UMVVM_LoadSlot>(this, LoadSlotViewModelClass);
	LoadSlotVM_0->SetLoadSlotName(FString("LoadSlot_0"));
	LoadSlotVM_1->SetLoadSlotName(FString("LoadSlot_1"));
	LoadSlotVM_2->SetLoadSlotName(FString("LoadSlot_2"));
	LoadSlotVM_0->SlotIndex = 0;
	LoadSlotVM_1->SlotIndex = 1;
	LoadSlotVM_2->SlotIndex = 2;
	LoadSlotVMs.Add(0, LoadSlotVM_0);
	LoadSlotVMs.Add(1, LoadSlotVM_1);
	LoadSlotVMs.Add(2, LoadSlotVM_2);

	SetNumLoadSlots(LoadSlotVMs.Num());
}

UMVVM_LoadSlot* UMVVM_LoadScreen::GetLoadSlotViewModelByIndex(const int32 SlotIndex) const
{
	return LoadSlotVMs.FindChecked(SlotIndex);
}

void UMVVM_LoadScreen::NewSlotButtonPressed(int32 SlotIndex, const FString& EnteredName)
{
	ASeptentGameModeBase* SeptentGameMode = Cast<ASeptentGameModeBase>(UGameplayStatics::GetGameMode(this));

	if (!IsValid(SeptentGameMode))
	{
		GEngine->AddOnScreenDebugMessage(1, 15.f, FColor::Magenta, TEXT("Please switch to Single Player"));
		return;
	}
	
	LoadSlotVMs[SlotIndex]->SetPlayerName(EnteredName);
	LoadSlotVMs[SlotIndex]->SetMapName(SeptentGameMode->DefaultMapName);
	LoadSlotVMs[SlotIndex]->SetPlayerLevel(1);
	LoadSlotVMs[SlotIndex]->SlotStatus = Taken;
	LoadSlotVMs[SlotIndex]->PlayerStartTag = SeptentGameMode->DefaultPlayerStartTag;
	LoadSlotVMs[SlotIndex]->MapAssetName = SeptentGameMode->DefaultMap.ToSoftObjectPath().GetAssetName();
	SeptentGameMode->SaveSlotData(LoadSlotVMs[SlotIndex], SlotIndex);
	LoadSlotVMs[SlotIndex]->InitializeSlot();

	USeptentGameInstance* SeptentGameInstance = SeptentGameMode->GetGameInstance<USeptentGameInstance>();
	SeptentGameInstance->LoadSlotName = LoadSlotVMs[SlotIndex]->GetLoadSlotName();
	SeptentGameInstance->LoadSlotIndex = SlotIndex;
	SeptentGameInstance->PlayerStartTag = SeptentGameMode->DefaultPlayerStartTag;
}

void UMVVM_LoadScreen::NewGameButtonPressed(int32 SlotIndex)
{
	LoadSlotVMs[SlotIndex]->OnSetWidgetSwitcherIndex.Broadcast(1);
}

void UMVVM_LoadScreen::SelectSlotButtonPressed(int32 SlotIndex)
{
	OnSlotSelected.Broadcast();
	for (const TTuple<int32, UMVVM_LoadSlot*>& LoadSlot : LoadSlotVMs)
	{
		if (LoadSlot.Key == SlotIndex)
		{
			LoadSlot.Value->OnEnableSelectSlotButton.Broadcast(false);
		}
		else
		{
			LoadSlot.Value->OnEnableSelectSlotButton.Broadcast(true);
		}
	}
	SelectedSlot = LoadSlotVMs[SlotIndex];
}

void UMVVM_LoadScreen::DeleteButtonPressed()
{
	if (IsValid(SelectedSlot))
	{
		ASeptentGameModeBase::DeleteSlotData(SelectedSlot, SelectedSlot->SlotIndex);
		SelectedSlot->SlotStatus = Vacant;
		SelectedSlot->InitializeSlot();
		SelectedSlot->OnEnableSelectSlotButton.Broadcast(true);
	}
}

void UMVVM_LoadScreen::PlayButtonPressed()
{
	ASeptentGameModeBase* SeptentGameMode = Cast<ASeptentGameModeBase>(UGameplayStatics::GetGameMode(this));
	USeptentGameInstance* SeptentGameInstance = Cast<USeptentGameInstance>(SeptentGameMode->GetGameInstance());
	SeptentGameInstance->PlayerStartTag = SelectedSlot->PlayerStartTag;
	SeptentGameInstance->LoadSlotName = SelectedSlot->GetLoadSlotName();
	SeptentGameInstance->LoadSlotIndex = SelectedSlot->SlotIndex;
	
	if (IsValid(SelectedSlot))
	{
		SeptentGameMode->TravelToMap(SelectedSlot);
	}
}

void UMVVM_LoadScreen::LoadData()
{
	ASeptentGameModeBase* SeptentGameMode = Cast<ASeptentGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (!IsValid(SeptentGameMode)) return;
	for (const TTuple<int32, UMVVM_LoadSlot*>& LoadSlot : LoadSlotVMs)
	{
		ULoadScreenSaveGame* SaveObject = SeptentGameMode->GetSaveSlotData(LoadSlot.Value->GetLoadSlotName(), LoadSlot.Key);
		const FString& PlayerName = SaveObject->PlayerName;
		const FString& MapName = SaveObject->MapName;
		const TEnumAsByte<ESaveSlotStatus> SlotStatus = SaveObject->SlotStatus;
		LoadSlot.Value->SlotStatus = SlotStatus;
		LoadSlot.Value->SetPlayerName(PlayerName);
		LoadSlot.Value->SetMapName(MapName);
		LoadSlot.Value->SetPlayerLevel(SaveObject->PlayerLevel);
		LoadSlot.Value->PlayerStartTag = SaveObject->PlayerStartTag;
		LoadSlot.Value->InitializeSlot();
	}
}

void UMVVM_LoadScreen::SetNumLoadSlots(int32 InNumLoadSlots)
{
	UE_MVVM_SET_PROPERTY_VALUE(NumLoadSlots, InNumLoadSlots);
}
