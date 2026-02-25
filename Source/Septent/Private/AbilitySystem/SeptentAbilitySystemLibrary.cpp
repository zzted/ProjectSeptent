// ZZ


#include "AbilitySystem/SeptentAbilitySystemLibrary.h"

#include <queue>

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "SeptentAbilitySystemTypes.h"
#include "SeptentGameplayTags.h"
#include "Engine/OverlapResult.h"
#include "Game/SeptentGameModeBase.h"
#include "Game/LoadScreenSaveGame.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Player/SeptentPlayerState.h"
#include "UI/HUD/SeptentHUD.h"
#include "UI/WidgetController/SeptentWidgetController.h"

bool USeptentAbilitySystemLibrary::GetWidgetControllerParams(const UObject* WorldContextObject,
                                                             FWidgetControllerParams& OutWCParams, ASeptentHUD*& OutSeptentHUD)
{
	if (APlayerController* PlayerController = UGameplayStatics::GetPlayerController(WorldContextObject, 0))
	{
		OutSeptentHUD = Cast<ASeptentHUD>(PlayerController->GetHUD());
		if (OutSeptentHUD)
		{
			ASeptentPlayerState* PlayerState = PlayerController->GetPlayerState<ASeptentPlayerState>();
			UAbilitySystemComponent* AbilitySystemComponent = PlayerState->GetAbilitySystemComponent();
			UAttributeSet* AttributeSet = PlayerState->GetAttributeSet();
			OutWCParams.AbilitySystemComponent = AbilitySystemComponent;
			OutWCParams.AttributeSet = AttributeSet;
			OutWCParams.PlayerController = PlayerController;
			OutWCParams.PlayerState = PlayerState;
			return true;
		}
	}
	return false;
}

UOverlayWidgetController* USeptentAbilitySystemLibrary::GetOverlayWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WidgetControllerParams;
	ASeptentHUD* SeptentHUD = nullptr;
	if (GetWidgetControllerParams(WorldContextObject, WidgetControllerParams, SeptentHUD))
	{
		return SeptentHUD->GetOverlayWidgetController(WidgetControllerParams);
	}
	return nullptr;
}

UAttributeMenuWidgetController* USeptentAbilitySystemLibrary::GetAttributeMenuWidgetController(
	const UObject* WorldContextObject)
{
	FWidgetControllerParams WidgetControllerParams;
	ASeptentHUD* SeptentHUD = nullptr;
	if (GetWidgetControllerParams(WorldContextObject, WidgetControllerParams, SeptentHUD))
	{
		return SeptentHUD->GetAttributeMenuWidgetController(WidgetControllerParams);
	}
	return nullptr;
}

USpellMenuWidgetController* USeptentAbilitySystemLibrary::GetSpellMenuWidgetController(const UObject* WorldContextObject)
{
	FWidgetControllerParams WidgetControllerParams;
	ASeptentHUD* SeptentHUD = nullptr;
	if (GetWidgetControllerParams(WorldContextObject, WidgetControllerParams, SeptentHUD))
	{
		return SeptentHUD->GetSpellMenuWidgetController(WidgetControllerParams);
	}
	return nullptr;
}

void USeptentAbilitySystemLibrary::InitializeDefaultAttributes(const UObject* WorldContextObject, const ECharacterClass CharacterClass, const float Level, UAbilitySystemComponent* AbilitySystemComponent)
{
	const AActor* AvatarActor = AbilitySystemComponent->GetAvatarActor();

	const UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (!CharacterClassInfo) return;
	FCharacterClassDefaultInfo ClassDefaultInfo = CharacterClassInfo->GetDefaultInfo(CharacterClass);
	
	FGameplayEffectContextHandle PrimaryAttributesEffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	PrimaryAttributesEffectContextHandle.AddSourceObject(AvatarActor);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*AbilitySystemComponent->MakeOutgoingSpec(ClassDefaultInfo.PrimaryAttributes, Level, PrimaryAttributesEffectContextHandle).Data.Get());
	
	FGameplayEffectContextHandle SecondaryAttributesEffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	SecondaryAttributesEffectContextHandle.AddSourceObject(AvatarActor);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*AbilitySystemComponent->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes, Level, SecondaryAttributesEffectContextHandle).Data.Get());
	
	FGameplayEffectContextHandle VitalAttributesEffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	VitalAttributesEffectContextHandle.AddSourceObject(AvatarActor);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*AbilitySystemComponent->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, Level, VitalAttributesEffectContextHandle).Data.Get());
}

void USeptentAbilitySystemLibrary::InitializeDefaultAttributesFromSaveData(const UObject* WorldContextObject,
	UAbilitySystemComponent* AbilitySystemComponent, ULoadScreenSaveGame* SaveGame)
{
	const UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (!CharacterClassInfo) return; // return if on client
	const FSeptentGameplayTags& GameplayTags = FSeptentGameplayTags::Get();

	const AActor* SourceAvatarActor = AbilitySystemComponent->GetAvatarActor();

	FGameplayEffectContextHandle EffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(SourceAvatarActor);
	
	const FGameplayEffectSpecHandle SpecHandle = AbilitySystemComponent->MakeOutgoingSpec(CharacterClassInfo->PrimaryAttributes_SetByCaller, 1.f, EffectContextHandle);

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Strength, SaveGame->Strength);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Intelligence, SaveGame->Intelligence);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Resilience, SaveGame->Resilience);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Attributes_Primary_Vigor, SaveGame->Vigor);

	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*SpecHandle.Data);

	FGameplayEffectContextHandle SecondaryAttributesEffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	SecondaryAttributesEffectContextHandle.AddSourceObject(SourceAvatarActor);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*AbilitySystemComponent->MakeOutgoingSpec(CharacterClassInfo->SecondaryAttributes_Infinite, 1.f, SecondaryAttributesEffectContextHandle).Data.Get());
	
	FGameplayEffectContextHandle VitalAttributesEffectContextHandle = AbilitySystemComponent->MakeEffectContext();
	VitalAttributesEffectContextHandle.AddSourceObject(SourceAvatarActor);
	AbilitySystemComponent->ApplyGameplayEffectSpecToSelf(*AbilitySystemComponent->MakeOutgoingSpec(CharacterClassInfo->VitalAttributes, 1.f, VitalAttributesEffectContextHandle).Data.Get());
}

void USeptentAbilitySystemLibrary::GiveStartupAbilities(const UObject* WorldContextObject,
                                                     UAbilitySystemComponent* AbilitySystemComponent, ECharacterClass CharacterClass)
{
	
	const UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (!CharacterClassInfo) return;
	for (const TSubclassOf<UGameplayAbility> AbilityClass :  CharacterClassInfo->CommonAbilities)
	{
		FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, 1);
		AbilitySystemComponent->GiveAbility(AbilitySpec);
	}
	const FCharacterClassDefaultInfo& DefaultInfo = CharacterClassInfo->GetDefaultInfo(CharacterClass);
	for (const TSubclassOf<UGameplayAbility> AbilityClass : DefaultInfo.StartupAbilities)
	{
		if (AbilitySystemComponent->GetAvatarActor()->Implements<UCombatInterface>())
		{
			FGameplayAbilitySpec AbilitySpec = FGameplayAbilitySpec(AbilityClass, ICombatInterface::Execute_GetPlayerLevel(AbilitySystemComponent->GetAvatarActor()));
			AbilitySystemComponent->GiveAbility(AbilitySpec);
		}
	}
}

int32 USeptentAbilitySystemLibrary::GetXPRewardForClassAndLevel(const UObject* WorldContextObject,
	const ECharacterClass CharacterClass, const int32 Level)
{
	const UCharacterClassInfo* CharacterClassInfo = GetCharacterClassInfo(WorldContextObject);
	if (!CharacterClassInfo) return 0;

	const FCharacterClassDefaultInfo& Info = CharacterClassInfo->GetDefaultInfo(CharacterClass);
	const float XPWard = Info.XPReward.GetValueAtLevel(Level);

	return static_cast<int32>(XPWard);
}

TArray<FRotator> USeptentAbilitySystemLibrary::EvenlySpacedRotators(const FVector& Forward, const FVector& Axis,
	float Spread, int32 NumRotators)
{
	TArray<FRotator> Rotators;
	if (NumRotators > 1)
	{
		const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);

		const float DeltaSpread = Spread / (NumRotators - 1);
		for (int32 i = 0; i < NumRotators; ++i)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, Axis);
			Rotators.Add(Direction.Rotation());
		}
	}
	else
	{
		Rotators.Add(Forward.Rotation());
	}

	return Rotators;
}

TArray<FVector> USeptentAbilitySystemLibrary::EvenlyRotatedVectors(const FVector& Forward, const FVector& Axis,
	float Spread, int32 NumVectors)
{
	TArray<FVector> Vectors;
	if (NumVectors > 1)
	{
		const FVector LeftOfSpread = Forward.RotateAngleAxis(-Spread / 2.f, Axis);

		const float DeltaSpread = Spread / (NumVectors - 1);
		for (int32 i = 0; i < NumVectors; ++i)
		{
			const FVector Direction = LeftOfSpread.RotateAngleAxis(DeltaSpread * i, Axis);
			Vectors.Add(Direction);
		}
	}
	else
	{
		Vectors.Add(Forward);
	}

	return Vectors;
}

void USeptentAbilitySystemLibrary::SetIsRadialDamageEffectParam(FDamageEffectParams& DamageEffectParams,
	const bool bInIsRadialDamage, const float InnerRadius, const float OuterRadius, const FVector Origin)
{
	DamageEffectParams.bIsRadialDamage = bInIsRadialDamage;
	DamageEffectParams.RadialDamageOrigin = Origin;
	DamageEffectParams.RadialDamageInnerRadius = InnerRadius;
	DamageEffectParams.RadialDamageOuterRadius = OuterRadius;
}

void USeptentAbilitySystemLibrary::SetKnockbackDirection(FDamageEffectParams& DamageEffectParams,
	 FVector& KnockbackDirection, const float Magnitude)
{
	KnockbackDirection.Normalize();
	if (Magnitude > 0.f)
	{
		DamageEffectParams.KnockbackForce = KnockbackDirection * Magnitude;
	}
	else
	{
		DamageEffectParams.KnockbackForce = KnockbackDirection * DamageEffectParams.KnockbackForceMagnitude;
	}
}

void USeptentAbilitySystemLibrary::SetDeathImpulseDirection(FDamageEffectParams& DamageEffectParams,
	FVector& DeathImpulseDirection, const float Magnitude)
{
	DeathImpulseDirection.Normalize();
	if (Magnitude > 0.f)
	{
		DamageEffectParams.DeathImpulse = DeathImpulseDirection * Magnitude;
	}
	else
	{
		DamageEffectParams.DeathImpulse = DeathImpulseDirection * DamageEffectParams.DeathImpulseMagnitude;
	}
	
}

void USeptentAbilitySystemLibrary::SetTargetEffectParamsASC(FDamageEffectParams& DamageEffectParams,
	UAbilitySystemComponent* InASC)
{
	DamageEffectParams.TargetAbilitySystemComponent = InASC;
}

UCharacterClassInfo* USeptentAbilitySystemLibrary::GetCharacterClassInfo(const UObject* WorldContextObject)
{
	const ASeptentGameModeBase* SeptentGameModeBase = Cast<ASeptentGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (SeptentGameModeBase == nullptr) return nullptr;

	UCharacterClassInfo* CharacterClassInfo = SeptentGameModeBase->CharacterClassInfo;
	return CharacterClassInfo;
}

UAbilityInfo* USeptentAbilitySystemLibrary::GetAbilityInfo(const UObject* WorldContextObject)
{
	const ASeptentGameModeBase* SeptentGameModeBase = Cast<ASeptentGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (SeptentGameModeBase == nullptr) return nullptr;

	UAbilityInfo* AbilityInfo = SeptentGameModeBase->AbilityInfo;
	return AbilityInfo;
}

ULootTiers* USeptentAbilitySystemLibrary::GetLootTiers(const UObject* WorldContextObject)
{
	const ASeptentGameModeBase* SeptentGameModeBase = Cast<ASeptentGameModeBase>(UGameplayStatics::GetGameMode(WorldContextObject));
	if (SeptentGameModeBase == nullptr) return nullptr;

	ULootTiers* LootTiers = SeptentGameModeBase->LootTiers;
	return LootTiers;
}

FGameplayEffectContextHandle USeptentAbilitySystemLibrary::ApplyDamageEffectsFromDamageEffectParams(const FDamageEffectParams& DamageEffectParams)
{
	const FSeptentGameplayTags& GameplayTags = FSeptentGameplayTags::Get();

	FGameplayEffectContextHandle EffectContextHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeEffectContext();
	EffectContextHandle.AddSourceObject(DamageEffectParams.SourceAbilitySystemComponent->GetAvatarActor());
	SetDeathImpulse(EffectContextHandle, DamageEffectParams.DeathImpulse);
	SetKnockbackForce(EffectContextHandle, DamageEffectParams.KnockbackForce);
	
	SetIsRadialDamage(EffectContextHandle, DamageEffectParams.bIsRadialDamage);
	SetRadialDamageOrigin(EffectContextHandle, DamageEffectParams.RadialDamageOrigin);
	SetRadialDamageInnerRadius(EffectContextHandle, DamageEffectParams.RadialDamageInnerRadius);
	SetRadialDamageOuterRadius(EffectContextHandle, DamageEffectParams.RadialDamageOuterRadius);
	
	const FGameplayEffectSpecHandle SpecHandle = DamageEffectParams.SourceAbilitySystemComponent->MakeOutgoingSpec(
		DamageEffectParams.DamageGameplayEffectClass, DamageEffectParams.AbilityLevel, EffectContextHandle);

	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, DamageEffectParams.DamageTypeTag, DamageEffectParams.BaseDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Chance, DamageEffectParams.DebuffChance);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Duration, DamageEffectParams.DebuffDuration);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Damage, DamageEffectParams.DebuffDamage);
	UAbilitySystemBlueprintLibrary::AssignTagSetByCallerMagnitude(SpecHandle, GameplayTags.Debuff_Frequency, DamageEffectParams.DebuffFrequency);
	
	DamageEffectParams.SourceAbilitySystemComponent->ApplyGameplayEffectSpecToTarget(*SpecHandle.Data, DamageEffectParams.TargetAbilitySystemComponent);

	return EffectContextHandle;
}

bool USeptentAbilitySystemLibrary::IsBlockedHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return SeptentGameplayEffectContext->IsBlockedHit();
	}
	return false;
}

bool USeptentAbilitySystemLibrary::IsCriticalHit(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return SeptentGameplayEffectContext->IsCriticalHit();
	}
	return false;
}

bool USeptentAbilitySystemLibrary::IsSuccessfulDebuff(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return SeptentGameplayEffectContext->IsSuccessfulDebuff();
	}
	return false;
}

float USeptentAbilitySystemLibrary::GetDebuffDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return SeptentGameplayEffectContext->GetDebuffDamage();
	}
	return 0.f;
}

float USeptentAbilitySystemLibrary::GetDebuffDuration(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return SeptentGameplayEffectContext->GetDebuffDuration();
	}
	return 0.f;
}

float USeptentAbilitySystemLibrary::GetDebuffFrequency(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return SeptentGameplayEffectContext->GetDebuffFrequency();
	}
	return 0.f;
}

FGameplayTag USeptentAbilitySystemLibrary::GetDamageType(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		if (SeptentGameplayEffectContext->GetDamageType().IsValid())
		{
			return *SeptentGameplayEffectContext->GetDamageType();
		}
	}
	return FGameplayTag();
}

FVector USeptentAbilitySystemLibrary::GetDeathImpulse(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return SeptentGameplayEffectContext->GetDeathImpulse();
	}
	return FVector::ZeroVector;
}

FVector USeptentAbilitySystemLibrary::GetKnockbackForce(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return SeptentGameplayEffectContext->GetKnockbackForce();
	}
	return FVector::ZeroVector;
}

bool USeptentAbilitySystemLibrary::IsRadialDamage(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return SeptentGameplayEffectContext->IsRadialDamage();
	}
	return false;
}

float USeptentAbilitySystemLibrary::GetRadialDamageInnerRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return SeptentGameplayEffectContext->GetRadialDamageInnerRadius();
	}
	return 0.f;
}

float USeptentAbilitySystemLibrary::GetRadialDamageOuterRadius(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return SeptentGameplayEffectContext->GetRadialDamageOuterRadius();
	}
	return 0.f;
}

FVector USeptentAbilitySystemLibrary::GetRadialDamageOrigin(const FGameplayEffectContextHandle& EffectContextHandle)
{
	if (const FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<const FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		return SeptentGameplayEffectContext->GetRadialDamageOrigin();
	}
	return FVector::ZeroVector;
}

void USeptentAbilitySystemLibrary::SetIsSuccessfulDebuff(FGameplayEffectContextHandle& EffectContextHandle,
                                                      const bool bInIsSuccessfulDebuff)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetIsSuccessfulDebuff(bInIsSuccessfulDebuff);
	}
}

void USeptentAbilitySystemLibrary::SetDebuffDamage(FGameplayEffectContextHandle& EffectContextHandle, const float InDamage)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetDebuffDamage(InDamage);
	}
}

void USeptentAbilitySystemLibrary::SetDebuffDuration(FGameplayEffectContextHandle& EffectContextHandle,
                                                  const float InDuration)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetDebuffDuration(InDuration);
	}
}

void USeptentAbilitySystemLibrary::SetDebuffFrequency(FGameplayEffectContextHandle& EffectContextHandle,
	const float InFrequency)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetDebuffFrequency(InFrequency);
	}
}

void USeptentAbilitySystemLibrary::SetDamageType(FGameplayEffectContextHandle& EffectContextHandle,
	const FGameplayTag& InDamageType)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetDamageType(MakeShareable(new FGameplayTag(InDamageType)));
	}
}

void USeptentAbilitySystemLibrary::SetDeathImpulse(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InImpulse)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetDeathImpulse(InImpulse);
	}
}

void USeptentAbilitySystemLibrary::SetKnockbackForce(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InForce)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetKnockbackForce(InForce);
	}
}

void USeptentAbilitySystemLibrary::SetIsBlockedHit(FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsBlockedHit)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetIsBlockedHit(bInIsBlockedHit);
	}
}

void USeptentAbilitySystemLibrary::SetIsCriticalHit(FGameplayEffectContextHandle& EffectContextHandle, const bool bInIsCriticalHit)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetIsCriticalHit(bInIsCriticalHit);
	}
}

void USeptentAbilitySystemLibrary::SetIsRadialDamage(FGameplayEffectContextHandle& EffectContextHandle,
	const bool bInIsRadialDamage)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetIsRadialDamage(bInIsRadialDamage);
	}
}

void USeptentAbilitySystemLibrary::SetRadialDamageInnerRadius(FGameplayEffectContextHandle& EffectContextHandle,
	const float InRadialDamageInnerRadius)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetRadialDamageInnerRadius(InRadialDamageInnerRadius);
	}
}

void USeptentAbilitySystemLibrary::SetRadialDamageOuterRadius(FGameplayEffectContextHandle& EffectContextHandle,
	const float InRadialDamageOuterRadius)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetRadialDamageOuterRadius(InRadialDamageOuterRadius);
	}
}

void USeptentAbilitySystemLibrary::SetRadialDamageOrigin(FGameplayEffectContextHandle& EffectContextHandle,
	const FVector& InRadialDamageOrigin)
{
	if (FSeptentGameplayEffectContext* SeptentGameplayEffectContext = static_cast<FSeptentGameplayEffectContext*>(EffectContextHandle.Get()))
	{
		SeptentGameplayEffectContext->SetRadialDamageOrigin(InRadialDamageOrigin);
	}
}

void USeptentAbilitySystemLibrary::GetLivePlayersWithinRadius(const UObject* WorldContextObject,
                                                           TArray<AActor*>& OutOverlappingActors, const TArray<AActor*>& ActorsToIgnore, const float Radius,
                                                           const FVector& SphereOrigin)
{
	FCollisionQueryParams SphereParams;
	SphereParams.AddIgnoredActors(ActorsToIgnore);
	TArray<FOverlapResult> Overlaps;
	if (const UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		World->OverlapMultiByObjectType(Overlaps, SphereOrigin, FQuat::Identity, FCollisionObjectQueryParams(FCollisionObjectQueryParams::InitType::AllDynamicObjects), FCollisionShape::MakeSphere(Radius), SphereParams);
		for (const FOverlapResult& Overlap : Overlaps)
		{
			if (Overlap.GetActor()->Implements<UCombatInterface>() && !ICombatInterface::Execute_IsDead(Overlap.GetActor()))
			{
				OutOverlappingActors.AddUnique(Overlap.GetActor());
			}
		}
	}
}

void USeptentAbilitySystemLibrary::GetClosestTargets(
	const int32 MaxTargets,
	const TArray<AActor*>& InTargets,
	TArray<AActor*>& OutClosestTargets,
	const FVector& Origin)
{
	if (MaxTargets <= 0 || InTargets.Num() <= 0)
	{
		OutClosestTargets.Reset();
		return;
	}
	OutClosestTargets.Reset();
	if (InTargets.Num() <= MaxTargets)
	{
		OutClosestTargets = InTargets;
		return;
	}

	struct FCompare
	{
		const FVector* OriginPtr;

		explicit FCompare(const FVector* InOriginPtr)
			: OriginPtr(InOriginPtr)
		{}

		bool operator()(const AActor* A, const AActor* B) const
		{
			// Farthest actor gets higher priority in the heap
			const float DistA = FVector::DistSquared(*OriginPtr, A->GetActorLocation());
			const float DistB = FVector::DistSquared(*OriginPtr, B->GetActorLocation());
			return DistA < DistB; 
		}
	};

	std::priority_queue<AActor*, std::vector<AActor*>, FCompare> MaxHeap((FCompare(&Origin)));

	for (AActor* Target : InTargets)
	{
		if (!IsValid(Target)) continue;

		if (MaxHeap.size() < MaxTargets)
		{
			MaxHeap.push(Target);
		}
		else
		{
			const AActor* Farthest = MaxHeap.top();

			const float DistNew   = FVector::DistSquared(Origin, Target->GetActorLocation());
			const float DistFar   = FVector::DistSquared(Origin, Farthest->GetActorLocation());

			if (DistNew < DistFar)
			{
				MaxHeap.pop();
				MaxHeap.push(Target);
			}
		}
	}

	while (!MaxHeap.empty())
	{
		OutClosestTargets.Add(MaxHeap.top());
		MaxHeap.pop();
	}
}

bool USeptentAbilitySystemLibrary::IsNotFriend(const AActor* FirstActor, const AActor* SecondActor)
{
	const bool bBothArePlayers = FirstActor->ActorHasTag(FName("Player")) && SecondActor->ActorHasTag(FName("Player"));
	const bool bBothAreEnemies = FirstActor->ActorHasTag(FName("Enemy")) && SecondActor->ActorHasTag(FName("Enemy"));
	const bool bFriends = bBothArePlayers || bBothAreEnemies;
	return !bFriends;
}


