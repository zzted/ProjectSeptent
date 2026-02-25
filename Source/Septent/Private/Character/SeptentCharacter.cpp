// ZZ
#include "Character/SeptentCharacter.h"

#include "AbilitySystemComponent.h"
#include "SeptentGameplayTags.h"
#include "NiagaraComponent.h"
#include "AbilitySystem/SeptentAbilitySystemComponent.h"
#include "AbilitySystem/SeptentAbilitySystemLibrary.h"
#include "AbilitySystem/SeptentAttributeSet.h"
#include "AbilitySystem/Data/AbilityInfo.h"
#include "AbilitySystem/Data/LevelUpInfo.h"
#include "AbilitySystem/Debuff/DebuffNiagaraComponent.h"
#include "Camera/CameraComponent.h"
#include "Game/SeptentGameModeBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Player/SeptentPlayerController.h"
#include "Player/SeptentPlayerState.h"
#include "UI/HUD/SeptentHUD.h"

ASeptentCharacter::ASeptentCharacter()
{
	SpringArmComponent = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComponent"));
	SpringArmComponent->SetupAttachment(GetRootComponent());
	SpringArmComponent->SetUsingAbsoluteRotation(true);
	SpringArmComponent->bDoCollisionTest = false;

	CameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComponent"));
	CameraComponent->SetupAttachment(SpringArmComponent, USpringArmComponent::SocketName);
	CameraComponent->bUsePawnControlRotation = false;
	
	LevelUpNiagaraComponent = CreateDefaultSubobject<UNiagaraComponent>(TEXT("LevelUpNiagaraComponent"));
	LevelUpNiagaraComponent->SetupAttachment(GetRootComponent());
	LevelUpNiagaraComponent->bAutoActivate = false;
	
	GetCharacterMovement()->bOrientRotationToMovement = true;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->bSnapToPlaneAtStart = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	bUseControllerRotationYaw = false;

	CharacterClass = ECharacterClass::Elementalist;
}

void ASeptentCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Init ability actor info for the server
	InitAbilityActorInfo();
	LoadProgress();
	if (ASeptentGameModeBase* SeptentGameMode = Cast<ASeptentGameModeBase>(UGameplayStatics::GetGameMode(this)))
	{
		SeptentGameMode->LoadWorldState(GetWorld());
	}
}

void ASeptentCharacter::LoadProgress() const
{
	ASeptentGameModeBase* SeptentGameMode = Cast<ASeptentGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (IsValid(SeptentGameMode))
	{
		ULoadScreenSaveGame* SaveData = SeptentGameMode->RetrieveInGameSaveData();
		if (SaveData == nullptr) return;
		
		if (SaveData->bFirstTimeLoadIn)
		{
			InitializeDefaultAttributes();
			AddCharacterAbilities();
		}
		else
		{
			if (USeptentAbilitySystemComponent* SeptentASC = Cast<USeptentAbilitySystemComponent>(AbilitySystemComponent))
			{
				SeptentASC->AddCharacterAbilitiesFromSaveData(SaveData);
			}
			
			if (ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>())
			{
				SeptentPlayerState->SetLevel(SaveData->PlayerLevel);
				SeptentPlayerState->SetXP(SaveData->PlayerXP);
				SeptentPlayerState->SetAttributePoints(SaveData->AttributePoints);
				SeptentPlayerState->SetSpellPoints(SaveData->SpellPoints);
			}
			USeptentAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(this, AbilitySystemComponent, SaveData);
		}
	}
}

void ASeptentCharacter::OnRep_PlayerState()
{
	Super::OnRep_PlayerState();

	// init ability on actor when player state has been synced at client
	InitAbilityActorInfo();
}

void ASeptentCharacter::AddToXP_Implementation(int32 InXP)
{
	ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>();
	check(SeptentPlayerState);
	SeptentPlayerState->AddToXP(InXP);
}

void ASeptentCharacter::LevelUp_Implementation()
{
	MulticastLevelUpParticles(); // MulticastLevelUpParticles_Implementation() will just execute locally, MulticastLevelUpParticles() will notify clients and execute
}

void ASeptentCharacter::MulticastLevelUpParticles_Implementation() const
{
	if (IsValid(LevelUpNiagaraComponent))
	{
		const FVector CameraLocation = CameraComponent->GetComponentLocation();
		const FVector NiagaraLocation = LevelUpNiagaraComponent->GetComponentLocation();
		const FRotator ToCameraRotation = (CameraLocation - NiagaraLocation).Rotation();
		LevelUpNiagaraComponent->SetWorldRotation(ToCameraRotation);
		LevelUpNiagaraComponent->Activate(true);
	}
}

int32 ASeptentCharacter::GetXP_Implementation() const
{
	const ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>();
	check(SeptentPlayerState);
	return SeptentPlayerState->GetXP();
}

int32 ASeptentCharacter::FindLevelForXP_Implementation(int32 InXP) const
{
	const ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>();
	check(SeptentPlayerState);
	return SeptentPlayerState->LevelUpInfo->FindLevelForXP(InXP);
}

int32 ASeptentCharacter::GetAttributePointsReward_Implementation(int32 InPlayerLevel) const
{
	const ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>();
	check(SeptentPlayerState);
	return SeptentPlayerState->LevelUpInfo->LevelUpInformation[InPlayerLevel].AttributePointReward;
}

int32 ASeptentCharacter::GetSpellPointsReward_Implementation(int32 InPlayerLevel) const
{
	
	const ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>();
	check(SeptentPlayerState);
	return SeptentPlayerState->LevelUpInfo->LevelUpInformation[InPlayerLevel].SpellPointReward;
}

void ASeptentCharacter::AddToPlayerLevel_Implementation(int32 InPlayerLevel)
{
	ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>();
	check(SeptentPlayerState);
	SeptentPlayerState->AddToLevel(InPlayerLevel);

	if (USeptentAbilitySystemComponent* SeptentASC = Cast<USeptentAbilitySystemComponent>(GetAbilitySystemComponent()))
	{
		SeptentASC->UpdateAbilityStatuses(SeptentPlayerState->GetPlayerLevel());
	}
}

void ASeptentCharacter::AddToAttributePoints_Implementation(int32 InAttributePoints)
{
	ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>();
	check(SeptentPlayerState);
	SeptentPlayerState->AddToAttributePoints(InAttributePoints);
}

void ASeptentCharacter::AddToSpellPoints_Implementation(int32 InSpellPoints)
{
	ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>();
	check(SeptentPlayerState);
	SeptentPlayerState->AddToSpellPoints(InSpellPoints);
}

int32 ASeptentCharacter::GetAttributePoints_Implementation() const
{
	const ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>();
	check(SeptentPlayerState);
	return SeptentPlayerState->GetAttributePoints();
}

int32 ASeptentCharacter::GetSpellPoints_Implementation() const
{
	const ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>();
	check(SeptentPlayerState);
	return SeptentPlayerState->GetSpellPoints();
}

void ASeptentCharacter::ShowMagicCircle_Implementation(UMaterialInterface* DecalMaterial)
{
	if (ASeptentPlayerController* SeptentPlayerController = Cast<ASeptentPlayerController>(GetController()))
	{
		SeptentPlayerController->ShowMagicCircle(DecalMaterial);
		SeptentPlayerController->bShowMouseCursor = false;
	}
}

void ASeptentCharacter::HideMagicCircle_Implementation()
{
	if (ASeptentPlayerController* SeptentPlayerController = Cast<ASeptentPlayerController>(GetController()))
	{
		SeptentPlayerController->HideMagicCircle();
		SeptentPlayerController->bShowMouseCursor = true;
	}
}

void ASeptentCharacter::SaveProgress_Implementation(const FName& CheckpointTag)
{
	ASeptentGameModeBase* SeptentGameMode = Cast<ASeptentGameModeBase>(UGameplayStatics::GetGameMode(this));
	if (IsValid(SeptentGameMode))
	{
		ULoadScreenSaveGame* SaveData = SeptentGameMode->RetrieveInGameSaveData();
		if (SaveData == nullptr) return;
		SaveData->PlayerStartTag = CheckpointTag;

		if (ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>())
		{
			SaveData->PlayerLevel = SeptentPlayerState->GetPlayerLevel();
			SaveData->PlayerXP = SeptentPlayerState->GetXP();
			SaveData->AttributePoints = SeptentPlayerState->GetAttributePoints();
			SaveData->SpellPoints = SeptentPlayerState->GetSpellPoints();
		}

		SaveData->Strength = USeptentAttributeSet::GetStrengthAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Intelligence = USeptentAttributeSet::GetIntelligenceAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Resilience = USeptentAttributeSet::GetResilienceAttribute().GetNumericValue(GetAttributeSet());
		SaveData->Vigor = USeptentAttributeSet::GetVigorAttribute().GetNumericValue(GetAttributeSet());

		SaveData->bFirstTimeLoadIn = false;

		if (!HasAuthority()) return;

		USeptentAbilitySystemComponent* SeptentASC = Cast<USeptentAbilitySystemComponent>(AbilitySystemComponent);
		FForEachAbility SaveAbilityDelegate;
		SaveData->SavedAbilities.Empty();
		SaveAbilityDelegate.BindLambda([this, SeptentASC, &SaveData](const FGameplayAbilitySpec& AbilitySpec)
		{
			const FGameplayTag AbilityTag = SeptentASC->GetAbilityTagFromSpec(AbilitySpec);
			UAbilityInfo* AbilityInfo = USeptentAbilitySystemLibrary::GetAbilityInfo(this);
			FSeptentAbilityInfo Info = AbilityInfo->FindAbilityInfoForTag(AbilityTag);

			FSavedAbility SavedAbility;
			SavedAbility.GameplayAbility = Info.Ability;
			SavedAbility.AbilityLevel = AbilitySpec.Level;
			SavedAbility.AbilitySlot = SeptentASC->GetInputTagFromAbilityTag(AbilityTag);
			SavedAbility.AbilityStatus = SeptentASC->GetStatusTagFromAbilityTag(AbilityTag);
			SavedAbility.AbilityTag = AbilityTag;
			SavedAbility.AbilityType = Info.AbilityType;
			
			SaveData->SavedAbilities.AddUnique(SavedAbility);
		});
		
		SeptentASC->ForEachAbility(SaveAbilityDelegate);
		
		SeptentGameMode->SaveInGameProgressData(SaveData);
	}
}

int32 ASeptentCharacter::GetPlayerLevel_Implementation()
{
	const ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>();
	check(SeptentPlayerState);
	return SeptentPlayerState->GetPlayerLevel();
}

void ASeptentCharacter::Die(const FVector& DeathImpulse)
{
	Super::Die(DeathImpulse);

	FTimerDelegate DeathTimerDelegate;
	DeathTimerDelegate.BindLambda([this]()
	{
		ASeptentGameModeBase* SeptentGameMode = Cast<ASeptentGameModeBase>(UGameplayStatics::GetGameMode(this));
		if (SeptentGameMode)
		{
			SeptentGameMode->PlayerDied(this);
		}
	});
	GetWorldTimerManager().SetTimer(DeathTimerHandle, DeathTimerDelegate, DeathTime, false);
	CameraComponent->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform); // Prevent Camera from falling
}

void ASeptentCharacter::OnRep_Stunned()
{
	if (USeptentAbilitySystemComponent* SeptentASC = Cast<USeptentAbilitySystemComponent>(AbilitySystemComponent))
	{
		const FSeptentGameplayTags& GameplayTags = FSeptentGameplayTags::Get();
		FGameplayTagContainer BlockedTags;
		BlockedTags.AddTag(GameplayTags.Player_Block_CursorTrace);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputHeld);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputPressed);
		BlockedTags.AddTag(GameplayTags.Player_Block_InputReleased);
		if (bIsStunned)
		{
			SeptentASC->AddLooseGameplayTags(BlockedTags);
			StunDebuffNiagaraComponent->Activate();
		}
		else
		{
			SeptentASC->RemoveLooseGameplayTags(BlockedTags);
			StunDebuffNiagaraComponent->Deactivate();
		}
	}
}

void ASeptentCharacter::OnRep_Burned()
{
	if (bIsBurned)
	{
		BurnDebuffNiagaraComponent->Activate();
	}
	else
	{
		BurnDebuffNiagaraComponent->Deactivate();
	}
}

void ASeptentCharacter::InitAbilityActorInfo()
{
	ASeptentPlayerState* SeptentPlayerState = GetPlayerState<ASeptentPlayerState>();
	check(SeptentPlayerState);
	SeptentPlayerState->GetAbilitySystemComponent()->InitAbilityActorInfo(SeptentPlayerState, this);
	Cast<USeptentAbilitySystemComponent>(SeptentPlayerState->GetAbilitySystemComponent())->AbilityActorInfoSet();
	AbilitySystemComponent = SeptentPlayerState->GetAbilitySystemComponent();
	AttributeSet = SeptentPlayerState->GetAttributeSet();
	OnAscRegistered.Broadcast(AbilitySystemComponent);
	AbilitySystemComponent->RegisterGameplayTagEvent(FSeptentGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ASeptentCharacter::StunTagChanged);
	
	
	if (ASeptentPlayerController* SeptentPlayerController = Cast<ASeptentPlayerController>(GetController()))
	{
		if (ASeptentHUD* SeptentHUD = Cast<ASeptentHUD>(SeptentPlayerController->GetHUD()))
		{
			SeptentHUD->InitOverlay(SeptentPlayerController, SeptentPlayerState, AbilitySystemComponent, AttributeSet);
		}
	}
}
