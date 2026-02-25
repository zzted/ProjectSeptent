// ZZ


#include "Game/SeptentGameModeBase.h"

#include "EngineUtils.h"
#include "Septent/SeptentLogChannels.h"
#include "Game/SeptentGameInstance.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerStart.h"
#include "Interaction/SaveInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Serialization/ObjectAndNameAsStringProxyArchive.h"
#include "UI/ViewModel/MVVM_LoadSlot.h"

void ASeptentGameModeBase::TravelToMap(const UMVVM_LoadSlot* Slot)
{
	const FString SlotName = Slot->GetLoadSlotName();
	const int32 SlotIndex = Slot->SlotIndex;

	UGameplayStatics::OpenLevelBySoftObjectPtr(Slot, Maps.FindChecked(Slot->GetMapName()));
}

void ASeptentGameModeBase::SaveSlotData(const UMVVM_LoadSlot* LoadSlot, const int32 SlotIndex) const
{
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetLoadSlotName(), SlotIndex);
	}
	USaveGame* SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	LoadScreenSaveGame->PlayerName = LoadSlot->GetPlayerName();
	LoadScreenSaveGame->MapName = LoadSlot->GetMapName();
	LoadScreenSaveGame->MapAssetName = LoadSlot->MapAssetName;
	LoadScreenSaveGame->SlotStatus = Taken;
	LoadScreenSaveGame->PlayerStartTag = LoadSlot->PlayerStartTag;
	UGameplayStatics::SaveGameToSlot(LoadScreenSaveGame, LoadSlot->GetLoadSlotName(), SlotIndex);
}

void ASeptentGameModeBase::DeleteSlotData(const UMVVM_LoadSlot* LoadSlot, const int32 SlotIndex)
{
	if (UGameplayStatics::DoesSaveGameExist(LoadSlot->GetLoadSlotName(), SlotIndex))
	{
		UGameplayStatics::DeleteGameInSlot(LoadSlot->GetLoadSlotName(), SlotIndex);
	}
}

ULoadScreenSaveGame* ASeptentGameModeBase::GetSaveSlotData(const FString& SlotName, int32 SlotIndex) const
{
	USaveGame* SaveGameObject = nullptr;
	if (UGameplayStatics::DoesSaveGameExist(SlotName, SlotIndex))
	{
		SaveGameObject = UGameplayStatics::LoadGameFromSlot(SlotName, SlotIndex);
	}
	else
	{
		SaveGameObject = UGameplayStatics::CreateSaveGameObject(LoadScreenSaveGameClass);
	}
	ULoadScreenSaveGame* LoadScreenSaveGame = Cast<ULoadScreenSaveGame>(SaveGameObject);
	return LoadScreenSaveGame;
}

ULoadScreenSaveGame* ASeptentGameModeBase::RetrieveInGameSaveData() const
{
	const USeptentGameInstance* SeptentGameInstance = GetGameInstance<USeptentGameInstance>();
	const FString InGameSaveSlotName = SeptentGameInstance->LoadSlotName;
	const int32 InGameSaveSlotIndex = SeptentGameInstance->LoadSlotIndex;
	return GetSaveSlotData(InGameSaveSlotName, InGameSaveSlotIndex);
}

void ASeptentGameModeBase::SaveInGameProgressData(ULoadScreenSaveGame* SaveObject) const
{
	USeptentGameInstance* SeptentGameInstance = GetGameInstance<USeptentGameInstance>();
	const FString InGameSaveSlotName = SeptentGameInstance->LoadSlotName;
	const int32 InGameSaveSlotIndex = SeptentGameInstance->LoadSlotIndex;
	SeptentGameInstance->PlayerStartTag = SaveObject->PlayerStartTag;
	UGameplayStatics::SaveGameToSlot(SaveObject, InGameSaveSlotName, InGameSaveSlotIndex);
}

void ASeptentGameModeBase::SaveWorldState(UWorld* World, const FString& DestinationMapAssetName) const
{
	FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	USeptentGameInstance* SeptentGameInstance = GetGameInstance<USeptentGameInstance>();
	check(SeptentGameInstance);
	if (ULoadScreenSaveGame* SaveGame = GetSaveSlotData(SeptentGameInstance->LoadSlotName, SeptentGameInstance->LoadSlotIndex))
	{
		if (!SaveGame->HasMapWithName(WorldName))
		{
			FSavedMap NewSavedMap;
			NewSavedMap.MapAssetName = WorldName;
			SaveGame->SavedMaps.Add(NewSavedMap);
		}

		FSavedMap SavedMap = SaveGame->GetSavedMapWithMapName(WorldName);
		SavedMap.SavedActors.Empty(); // clear it out, we'll fill it in with actors

		for (FActorIterator It(World); It; ++It)
		{
			if (DestinationMapAssetName != FString(""))
			{
				SaveGame->MapAssetName = DestinationMapAssetName;
				SaveGame->MapName = GetMapNameFromMapAssetName(DestinationMapAssetName);
			}
			
			AActor* Actor = *It;
			if (!IsValid(Actor) || !Actor->Implements<USaveInterface>()) continue; // pending kill or not a saved actor

			FSavedActor SavedActor;
			SavedActor.ActorName = Actor->GetFName();
			SavedActor.ActorTransform = Actor->GetTransform();

			FMemoryWriter MemoryWriter(SavedActor.Bytes);

			FObjectAndNameAsStringProxyArchive Archive(MemoryWriter, true);
			Archive.ArIsSaveGame = true;

			Actor->Serialize(Archive);
			
			SavedMap.SavedActors.AddUnique(SavedActor);
		}

		for (FSavedMap& MapToReplace : SaveGame->SavedMaps)
		{
			if (MapToReplace.MapAssetName == WorldName)
			{
				MapToReplace = SavedMap;
			}
		}

		UGameplayStatics::SaveGameToSlot(SaveGame, SeptentGameInstance->LoadSlotName, SeptentGameInstance->LoadSlotIndex);
	}
}

void ASeptentGameModeBase::LoadWorldState(UWorld* World) const
{
	FString WorldName = World->GetMapName();
	WorldName.RemoveFromStart(World->StreamingLevelsPrefix);

	USeptentGameInstance* SeptentGameInstance = GetGameInstance<USeptentGameInstance>();
	check(SeptentGameInstance);

	if (UGameplayStatics::DoesSaveGameExist(SeptentGameInstance->LoadSlotName, SeptentGameInstance->LoadSlotIndex))
	{
		ULoadScreenSaveGame* SaveGame = Cast<ULoadScreenSaveGame>(UGameplayStatics::LoadGameFromSlot(SeptentGameInstance->LoadSlotName, SeptentGameInstance->LoadSlotIndex));
		if (SaveGame == nullptr)
		{
			UE_LOG(LogSeptent, Error, TEXT("Failed to load save game for slot %s"), *(SeptentGameInstance->LoadSlotName + ""));
			return;
		}
		
		for (FActorIterator It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor->Implements<USaveInterface>()) continue;
			
			for (FSavedActor& SavedActor : SaveGame->GetSavedMapWithMapName(WorldName).SavedActors)
			{
				if (SavedActor.ActorName == Actor->GetFName())
				{
					if (ISaveInterface::Execute_ShouldLoadTransform(Actor))
					{
						Actor->SetActorTransform(SavedActor.ActorTransform);
					}

					FMemoryReader MemoryReader(SavedActor.Bytes);
					FObjectAndNameAsStringProxyArchive Archive(MemoryReader, true);
					Archive.ArIsSaveGame = true;
					Actor->Serialize(Archive); // converts binary bytes back into variables

					ISaveInterface::Execute_LoadActor(Actor);
				}
			}
		}
	}
}

FString ASeptentGameModeBase::GetMapNameFromMapAssetName(const FString& MapAssetName) const
{
	for (const TPair<FString, TSoftObjectPtr<UWorld>>& Map : Maps)
	{
		if (Map.Value.ToSoftObjectPath().GetAssetName() == MapAssetName)
		{
			return Map.Key;
		}
	}
	return FString();
}

AActor* ASeptentGameModeBase::ChoosePlayerStart_Implementation(AController* Player)
{
	const USeptentGameInstance* SeptentGameInstance = GetGameInstance<USeptentGameInstance>();
	TArray<AActor*> Actors;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), Actors);
	if (Actors.Num() > 0)
	{
		AActor* SelectedActor = Actors[0];
		for (AActor* Actor : Actors)
		{
			if (APlayerStart* PlayerStart = Cast<APlayerStart>(Actor))
			{
				if (PlayerStart->PlayerStartTag == SeptentGameInstance->PlayerStartTag)
				{
					SelectedActor = PlayerStart;
					break;
				}
			}
		}
		return SelectedActor;
	}
	return nullptr;
}

void ASeptentGameModeBase::PlayerDied(ACharacter* DeadCharacter)
{
	ULoadScreenSaveGame* SaveGame = RetrieveInGameSaveData();
	if (!IsValid(SaveGame)) return;
	UGameplayStatics::OpenLevel(DeadCharacter, FName(SaveGame->MapAssetName));
}

void ASeptentGameModeBase::BeginPlay()
{
	Super::BeginPlay();
	Maps.Add(DefaultMapName, DefaultMap);
}
