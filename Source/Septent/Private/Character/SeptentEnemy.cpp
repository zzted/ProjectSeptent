// ZZ


#include "Character/SeptentEnemy.h"

#include "SeptentGameplayTags.h"
#include "AbilitySystem/SeptentAbilitySystemComponent.h"
#include "AbilitySystem/SeptentAbilitySystemLibrary.h"
#include "AbilitySystem/SeptentAttributeSet.h"
#include "AI/SeptentAIController.h"
#include "Septent/Septent.h"
#include "BehaviorTree/BehaviorTree.h"
#include "BehaviorTree/BlackboardComponent.h"
#include "Components/WidgetComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UI/Widget/SeptentUserWidget.h"

ASeptentEnemy::ASeptentEnemy()
{
	GetMesh()->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);

	AbilitySystemComponent = CreateDefaultSubobject<USeptentAbilitySystemComponent>("AbilitySystemComponent");
	AbilitySystemComponent->SetIsReplicated(true);
	AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bUseControllerDesiredRotation = true;

	AttributeSet = CreateDefaultSubobject<USeptentAttributeSet>("AttributeSet");
	HealthBar = CreateDefaultSubobject<UWidgetComponent>("HealthBar");
	HealthBar->SetupAttachment(GetRootComponent());

	GetMesh()->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	GetMesh()->MarkRenderStateDirty();
	Weapon->SetCustomDepthStencilValue(CUSTOM_DEPTH_RED);
	Weapon->MarkRenderStateDirty();
	
	BaseWalkSpeed = 250.f;
}

void ASeptentEnemy::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (!HasAuthority()) return; // AI only matters on server
	SeptentAIController = Cast<ASeptentAIController>(NewController);
	SeptentAIController->GetBlackboardComponent()->InitializeBlackboard(*BehaviorTree->BlackboardAsset);
	SeptentAIController->RunBehaviorTree(BehaviorTree);
	SeptentAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), false);
	SeptentAIController->GetBlackboardComponent()->SetValueAsBool(FName("RangedAttacker"), CharacterClass != ECharacterClass::Warrior);

}

void ASeptentEnemy::HighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(true);
	Weapon->SetRenderCustomDepth(true);
}

void ASeptentEnemy::UnHighlightActor_Implementation()
{
	GetMesh()->SetRenderCustomDepth(false);
	Weapon->SetRenderCustomDepth(false);
}

void ASeptentEnemy::SetMoveToLocation_Implementation(FVector& OutDestination)
{
	// Do not change OutDestination
}

int32 ASeptentEnemy::GetPlayerLevel_Implementation()
{
	return Level;
}

void ASeptentEnemy::Die(const FVector& DeathImpulse)
{
	SetLifeSpan(LifeSpan);
	if (SeptentAIController)
	{
		SeptentAIController->GetBlackboardComponent()->SetValueAsBool(FName("Dead"), true);
	}
	SpawnLoot();
	Super::Die(DeathImpulse);
}

void ASeptentEnemy::SetCombatTarget_Implementation(AActor* InCombatTarget)
{
	CombatTarget = InCombatTarget;
}

AActor* ASeptentEnemy::GetCombatTarget_Implementation() const
{
	return CombatTarget;
}

void ASeptentEnemy::BeginPlay()
{
	Super::BeginPlay();
	InitAbilityActorInfo();
	GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	if (HasAuthority())
	{
		USeptentAbilitySystemLibrary::GiveStartupAbilities(this, AbilitySystemComponent, CharacterClass);
	}
	
	if (USeptentUserWidget* SeptentUserWidget = Cast<USeptentUserWidget>(HealthBar->GetUserWidgetObject()))
	{
		SeptentUserWidget->SetWidgetController(this);
	}
	
	if (const USeptentAttributeSet* SeptentAttributeSetSet = CastChecked<USeptentAttributeSet>(AttributeSet))
	{
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(SeptentAttributeSetSet->GetHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnHealthChanged.Broadcast(Data.NewValue);
			}
		);
		AbilitySystemComponent->GetGameplayAttributeValueChangeDelegate(SeptentAttributeSetSet->GetMaxHealthAttribute()).AddLambda(
			[this](const FOnAttributeChangeData& Data)
			{
				OnMaxHealthChanged.Broadcast(Data.NewValue);
			}
		);

		AbilitySystemComponent->RegisterGameplayTagEvent(FSeptentGameplayTags::Get().Effects_HitReact, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ASeptentEnemy::HitReactTagChanged);

		OnHealthChanged.Broadcast(SeptentAttributeSetSet->GetHealth()); // Enemy itself is the widget controller, that's why it's broadcast here
		OnMaxHealthChanged.Broadcast(SeptentAttributeSetSet->GetMaxHealth());
	}
}

void ASeptentEnemy::HitReactTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	bHitReacting = NewCount > 0;
	GetCharacterMovement()->MaxWalkSpeed = bHitReacting ? 0.f : BaseWalkSpeed;
	if (SeptentAIController && SeptentAIController->GetBlackboardComponent())
	{
		SeptentAIController->GetBlackboardComponent()->SetValueAsBool(FName("HitReacting"), bHitReacting);
	}
}

void ASeptentEnemy::InitAbilityActorInfo()
{
	AbilitySystemComponent->InitAbilityActorInfo(this, this);
	Cast<USeptentAbilitySystemComponent>(AbilitySystemComponent)->AbilityActorInfoSet();
	AbilitySystemComponent->RegisterGameplayTagEvent(FSeptentGameplayTags::Get().Debuff_Stun, EGameplayTagEventType::NewOrRemoved).AddUObject(this, &ASeptentEnemy::StunTagChanged);
	if (HasAuthority())
	{
		InitializeDefaultAttributes();
	}
	OnAscRegistered.Broadcast(AbilitySystemComponent);
}

void ASeptentEnemy::InitializeDefaultAttributes() const
{
	USeptentAbilitySystemLibrary::InitializeDefaultAttributes(this, CharacterClass, Level, AbilitySystemComponent);
}

void ASeptentEnemy::StunTagChanged(const FGameplayTag CallbackTag, int32 NewCount)
{
	Super::StunTagChanged(CallbackTag, NewCount);
	if (SeptentAIController && SeptentAIController->GetBlackboardComponent())
	{
		SeptentAIController->GetBlackboardComponent()->SetValueAsBool(FName("Stunned"), bIsStunned);
	}
}
