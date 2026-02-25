// ZZ


#include "AbilitySystem/ExecCalc/ExecCalc_Damage.h"

#include "AbilitySystemComponent.h"
#include "SeptentAbilitySystemTypes.h"
#include "SeptentGameplayTags.h"
#include "AbilitySystem/SeptentAbilitySystemLibrary.h"
#include "AbilitySystem/SeptentAttributeSet.h"
#include "AbilitySystem/Data/CharacterClassInfo.h"
#include "Septent/SeptentLogChannels.h"
#include "HLSLTree/HLSLTreeTypes.h"
#include "Interaction/CombatInterface.h"
#include "Kismet/GameplayStatics.h"

struct SeptentDamageStatics
{
	// Similar to what we did in MMC but using builtin Macros instead
	DECLARE_ATTRIBUTE_CAPTUREDEF(Armor);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArmorPenetration);
	DECLARE_ATTRIBUTE_CAPTUREDEF(BlockChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitChance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(CriticalHitResistance);
	
	DECLARE_ATTRIBUTE_CAPTUREDEF(FireResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(LightningResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(ArcaneResistance);
	DECLARE_ATTRIBUTE_CAPTUREDEF(PhysicalResistance);

	TMap<FGameplayTag, FGameplayEffectAttributeCaptureDefinition> TagsToCaptureDefs;

	
	SeptentDamageStatics()
	{
		DEFINE_ATTRIBUTE_CAPTUREDEF(USeptentAttributeSet, Armor, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USeptentAttributeSet, ArmorPenetration, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USeptentAttributeSet, BlockChance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USeptentAttributeSet, CriticalHitChance, Source, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USeptentAttributeSet, CriticalHitResistance, Target, false);
		
		DEFINE_ATTRIBUTE_CAPTUREDEF(USeptentAttributeSet, FireResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USeptentAttributeSet, LightningResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USeptentAttributeSet, ArcaneResistance, Target, false);
		DEFINE_ATTRIBUTE_CAPTUREDEF(USeptentAttributeSet, PhysicalResistance, Target, false);

		TagsToCaptureDefs.Add(FSeptentGameplayTags::Get().Attributes_Secondary_Armor, ArmorDef);
		TagsToCaptureDefs.Add(FSeptentGameplayTags::Get().Attributes_Secondary_ArmorPenetration, ArmorPenetrationDef);
		TagsToCaptureDefs.Add(FSeptentGameplayTags::Get().Attributes_Secondary_BlockChance, BlockChanceDef);
		TagsToCaptureDefs.Add(FSeptentGameplayTags::Get().Attributes_Secondary_CriticalHitChance, CriticalHitChanceDef);
		TagsToCaptureDefs.Add(FSeptentGameplayTags::Get().Attributes_Secondary_CriticalHitResistance, CriticalHitResistanceDef);
		
		TagsToCaptureDefs.Add(FSeptentGameplayTags::Get().Attributes_Resistance_Fire, FireResistanceDef);
		TagsToCaptureDefs.Add(FSeptentGameplayTags::Get().Attributes_Resistance_Lightning, LightningResistanceDef);
		TagsToCaptureDefs.Add(FSeptentGameplayTags::Get().Attributes_Resistance_Arcane, ArcaneResistanceDef);
		TagsToCaptureDefs.Add(FSeptentGameplayTags::Get().Attributes_Resistance_Physical, PhysicalResistanceDef);
	}
};

static const SeptentDamageStatics& DamageStatics()
{
	static SeptentDamageStatics DStatics;
	return DStatics;
}

UExecCalc_Damage::UExecCalc_Damage()
{
	RelevantAttributesToCapture.Add(DamageStatics().ArmorDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArmorPenetrationDef);
	RelevantAttributesToCapture.Add(DamageStatics().BlockChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitChanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().CriticalHitResistanceDef);
	
	RelevantAttributesToCapture.Add(DamageStatics().FireResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().LightningResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().ArcaneResistanceDef);
	RelevantAttributesToCapture.Add(DamageStatics().PhysicalResistanceDef);
}

void UExecCalc_Damage::DetermineDebuffs(const FGameplayEffectCustomExecutionParameters& ExecutionParams, const FGameplayEffectSpec& Spec, const FAggregatorEvaluateParameters& EvaluationParameters) const
{
	const FSeptentGameplayTags& GameplayTags = FSeptentGameplayTags::Get();
	for (TTuple<FGameplayTag, FGameplayTag> Pair : GameplayTags.DamageTypesToDebuffs)
	{
		const FGameplayTag& DamageTypeTag = Pair.Key;
		const FGameplayTag& DebuffTag = Pair.Value;
		const float TypeDamage = Spec.GetSetByCallerMagnitude(DamageTypeTag, false, -1.f);
		if (TypeDamage > -.5f) // .5 padding for floating point precision
		{
			// Determine if there's a successful debuff
			const float SourceDebuffChance = Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Chance, false, -1.f);

			float TargetDebuffResistance = 0.f;
			const FGameplayTag& ResistanceTag = GameplayTags.DamageTypesToResistances[DamageTypeTag];
			ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().TagsToCaptureDefs[ResistanceTag], EvaluationParameters, TargetDebuffResistance);
			TargetDebuffResistance = FMath::Max<float>(TargetDebuffResistance, 0.0f);
			const float EffectiveDebuffChance = SourceDebuffChance * ( 100 - TargetDebuffResistance ) / 100.f;
			const bool bDebuff = FMath::RandRange(1, 100) < EffectiveDebuffChance;
			UE_LOG(LogSeptent, Log, TEXT("bIsSuccessfulDebuff %hhd"), bDebuff);
			if (bDebuff)
			{
				FGameplayEffectContextHandle EffectContextHandle = Spec.GetContext();
				USeptentAbilitySystemLibrary::SetIsSuccessfulDebuff(EffectContextHandle, true);
				USeptentAbilitySystemLibrary::SetDamageType(EffectContextHandle, DamageTypeTag);
				USeptentAbilitySystemLibrary::SetDebuffDamage(EffectContextHandle, Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Damage, false, -1.f));
				USeptentAbilitySystemLibrary::SetDebuffDuration(EffectContextHandle, Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Duration, false, -1.f));
				USeptentAbilitySystemLibrary::SetDebuffFrequency(EffectContextHandle, Spec.GetSetByCallerMagnitude(GameplayTags.Debuff_Frequency, false, -1.f));
			}
		}
	}
}

void UExecCalc_Damage::Execute_Implementation(const FGameplayEffectCustomExecutionParameters& ExecutionParams,
                                              FGameplayEffectCustomExecutionOutput& OutExecutionOutput) const
{
	const FSeptentGameplayTags& GameplayTags = FSeptentGameplayTags::Get();
	const UAbilitySystemComponent* SourceAbilitySystemComponent = ExecutionParams.GetSourceAbilitySystemComponent();
	const UAbilitySystemComponent* TargetAbilitySystemComponent = ExecutionParams.GetTargetAbilitySystemComponent();

	AActor* SourceAvatar = SourceAbilitySystemComponent ? SourceAbilitySystemComponent->GetAvatarActor() : nullptr;
	AActor* TargetAvatar = TargetAbilitySystemComponent ? TargetAbilitySystemComponent->GetAvatarActor() : nullptr;


	int32 SourcePlayerLevel = 1;
	if (SourceAvatar->Implements<UCombatInterface>())
	{
		SourcePlayerLevel = ICombatInterface::Execute_GetPlayerLevel(SourceAvatar);
	}

	int32 TargetPlayerLevel = 1;
	if (TargetAvatar->Implements<UCombatInterface>())
	{
		TargetPlayerLevel = ICombatInterface::Execute_GetPlayerLevel(TargetAvatar);
	}

	const FGameplayEffectSpec& Spec = ExecutionParams.GetOwningSpec();
	FGameplayEffectContextHandle ContextHandle = Spec.GetContext();

	FAggregatorEvaluateParameters EvaluationParameters;
	EvaluationParameters.SourceTags = Spec.CapturedSourceTags.GetAggregatedTags();
	EvaluationParameters.TargetTags = Spec.CapturedTargetTags.GetAggregatedTags();

	// Debuff

	DetermineDebuffs(ExecutionParams, Spec, EvaluationParameters);

	// Get Damage Set by Caller Magnitude

	float Damage = 0.f;

	for (const TPair<FGameplayTag, FGameplayTag>& Pair : GameplayTags.DamageTypesToResistances)
	{
		const FGameplayTag& DamageTypeTag = Pair.Key;
		const FGameplayTag& ResistanceTag = Pair.Value;
		const FGameplayEffectAttributeCaptureDefinition& Def = DamageStatics().TagsToCaptureDefs.FindRef(ResistanceTag);
		float DamageTypeValue = Spec.GetSetByCallerMagnitude(DamageTypeTag, false);
		if (DamageTypeValue <= 0.f) continue;;

		if (USeptentAbilitySystemLibrary::IsRadialDamage(ContextHandle))
		{
			// 1. override TakeDamage in SeptentCharacterBase
			// 2. create delegate OnDamageDelegate, broadcast damage received in TakeDamage
			// 3. bind lambda to OnDamageDelegate on the Victim here
			// 4. Call UGameplayStatics::ApplyRadialDamageWithFalloff to cause damage (this will result in TakeDamage being called on the Victim which will then broadcast OnDamageDelegate
			// 5. in lambda set DamageTypeValue to the damage received from the broadcast
			if (ICombatInterface* CombatInterface = Cast<ICombatInterface>(TargetAvatar))
			{
				CombatInterface->GetOnDamageSignature().AddLambda([&](float DamageAmount)
				{
					DamageTypeValue = DamageAmount;
				});
			}

			FVector DamageOrigin = USeptentAbilitySystemLibrary::GetRadialDamageOrigin(ContextHandle);
			DamageOrigin.Z += 200.f;
			UGameplayStatics::ApplyRadialDamageWithFalloff(
				TargetAvatar,
				DamageTypeValue,
				0.f,
				DamageOrigin,
				USeptentAbilitySystemLibrary::GetRadialDamageInnerRadius(ContextHandle),
				USeptentAbilitySystemLibrary::GetRadialDamageOuterRadius(ContextHandle),
				1.f,
				UDamageType::StaticClass(),
				TArray<AActor*>(),
				SourceAvatar,
				nullptr); // Need to change the object type to world dynamic for the function to work
		}
		
		float Resistance = 0.f;
		ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(Def, EvaluationParameters, Resistance);
		Resistance = FMath::Max<float>(Resistance, 0.0f);
		Resistance = FMath::Min<float>(Resistance, 100.0f);
		
		Damage += DamageTypeValue * (100.f - Resistance) / 100.f;
	}
	
	// 1.5 The Damage if is a crit hit

	float CriticalHitChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitChanceDef, EvaluationParameters, CriticalHitChance);
	CriticalHitChance = FMath::Max<float>(CriticalHitChance, 0.0f);

	float CriticalHitResistance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().CriticalHitResistanceDef, EvaluationParameters, CriticalHitResistance);
	CriticalHitResistance = FMath::Max<float>(CriticalHitResistance, 0.0f);

	if (FMath::RandRange(0.f, 1.f) <= CriticalHitChance)
	{
		Damage *= 1.0 + 0.5 * FMath::Max<float>(1.0 - CriticalHitResistance, 0.f);
		USeptentAbilitySystemLibrary::SetIsCriticalHit(ContextHandle, true);
	}

	// Capture BlockChance on Target and determine if there's a successful block
	
	float BlockChance = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().BlockChanceDef, EvaluationParameters, BlockChance);
	BlockChance = FMath::Max<float>(BlockChance, 0.0f);

	// If block, halve the damage
	if (FMath::RandRange(0.f, 1.f) <= BlockChance)
	{
		Damage *= 0.5f;
		USeptentAbilitySystemLibrary::SetIsBlockedHit(ContextHandle, true);
	}

	// ArmorPenetration ignores a percentage of the Target's Armor

	float TargetArmor = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorDef, EvaluationParameters, TargetArmor);
	TargetArmor = FMath::Max<float>(TargetArmor, 0.0f);

	float SourceArmorPen = 0.f;
	ExecutionParams.AttemptCalculateCapturedAttributeMagnitude(DamageStatics().ArmorPenetrationDef, EvaluationParameters, SourceArmorPen);
	SourceArmorPen = FMath::Max<float>(SourceArmorPen, 0.0f);

	const UCharacterClassInfo* CharacterClassInfo = USeptentAbilitySystemLibrary::GetCharacterClassInfo(SourceAvatar);
	const FRealCurve* ArmorPenetrationCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("ArmorPenetration"), FString());
	const FRealCurve* EffectiveArmorCurve = CharacterClassInfo->DamageCalculationCoefficients->FindCurve(FName("EffectiveArmor"), FString());
	const float ArmorPenetrationCoefficient = ArmorPenetrationCurve->Eval(SourcePlayerLevel);
	const float EffectiveArmorCoefficient = EffectiveArmorCurve->Eval(TargetPlayerLevel);
	
	
	// Armor ignores a percentage of damage
	const float EffectiveArmor = FMath::Max<float>(TargetArmor * (100.f - SourceArmorPen * ArmorPenetrationCoefficient) / 100.f, 0.f);
	Damage *= (100 - EffectiveArmor * EffectiveArmorCoefficient) / 100.f;
	
	// Meta-Attribute IncomingDamage is set here
	const FGameplayModifierEvaluatedData EvalData(USeptentAttributeSet::GetIncomingDamageAttribute(), EGameplayModOp::Additive, Damage);
                                                   	OutExecutionOutput.AddOutputModifier(EvalData);
}
