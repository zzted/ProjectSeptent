// ZZ


#include "Player/SeptentPlayerController.h"

#include "AbilitySystemBlueprintLibrary.h"
#include "AbilitySystemComponent.h"
#include "GameFramework/Character.h"
#include "SeptentGameplayTags.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "NavigationPath.h"
#include "NavigationSystem.h"
#include "NiagaraFunctionLibrary.h"
#include "AbilitySystem/SeptentAbilitySystemComponent.h"
#include "Actor/MagicCircle.h"
#include "Septent/Septent.h"
#include "Components/DecalComponent.h"
#include "Components/SplineComponent.h"
#include "Input/SeptentInputComponent.h"
#include "Interaction/EnemyInterface.h"
#include "Interaction/HighlightInterface.h"
#include "Net/RepLayout.h"
#include "UI/Widget/DamageTextComponent.h"

ASeptentPlayerController::ASeptentPlayerController()
{
	bReplicates = true;

	Spline = CreateDefaultSubobject<USplineComponent>("Spline");
}

void ASeptentPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);
	CursorTrace();
	AutoRun();
	UpdateMagicCircleLocation();
}

void ASeptentPlayerController::ShowMagicCircle(UMaterialInterface* DecalMaterial)
{
	if (!IsValid(MagicCircle))
	{
		MagicCircle = GetWorld()->SpawnActor<AMagicCircle>(MagicCircleClass);
		if (DecalMaterial)
		{
			MagicCircle->MagicCircleDecal->SetMaterial(0, DecalMaterial);
		}
	}
}

void ASeptentPlayerController::HideMagicCircle()
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->Destroy();
	}
}

void ASeptentPlayerController::ShowDamageNumber_Implementation(const float DamageAmount, ACharacter* TargetCharacter, const bool bBlockedHit, const bool bCriticalHit)
{
	if (IsValid(TargetCharacter) && DamageTextComponentClass && IsLocalController()) // Character is a pointer but also might be pending kill or GC that's why we need IsValid to perform the check
		// Using IsLocalController() to show text only on the client that's doing the damage
	{
		UDamageTextComponent* DamageText = NewObject<UDamageTextComponent>(TargetCharacter, DamageTextComponentClass);
		DamageText->RegisterComponent(); // manually registering because we're creating Widget dynamically instead of using CreateDefaultSubobject which does the registration for us
		DamageText->AttachToComponent(TargetCharacter->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		DamageText->DetachFromComponent(FDetachmentTransformRules::KeepWorldTransform);
		DamageText->SetDamageText(DamageAmount, bBlockedHit, bCriticalHit);
	}
}


void ASeptentPlayerController::AutoRun()
{
	if (!bAutoRunning) return;
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		const FVector LocationOnSpline = Spline->FindLocationClosestToWorldLocation(ControlledPawn->GetActorLocation(), ESplineCoordinateSpace::World);
		const FVector Direction = Spline->FindDirectionClosestToWorldLocation(LocationOnSpline, ESplineCoordinateSpace::World);
		ControlledPawn->AddMovementInput(Direction, 1.f);

		const float DistanceToDestination = (LocationOnSpline - CachedDestination).Length();
		if (DistanceToDestination <= AutoRunAcceptanceRadius)
		{
			bAutoRunning = false;
		}
	}
}

void ASeptentPlayerController::UpdateMagicCircleLocation() const
{
	if (IsValid(MagicCircle))
	{
		MagicCircle->SetActorLocation(CursorHit.ImpactPoint);
	}
}

void ASeptentPlayerController::CursorTrace()
{
	if (GetAbilitySystemComponent() && GetAbilitySystemComponent()->HasMatchingGameplayTag(FSeptentGameplayTags::Get().Player_Block_CursorTrace))
	{
		UnHighLightActor(LastActor);
		UnHighLightActor(ThisActor);
		LastActor = nullptr;
		ThisActor = nullptr;
		return;
	}
	const ECollisionChannel TraceChannel = IsValid(MagicCircle) ? ECC_ExcludeCharacters : ECC_Visibility; // If MagicCircle is valid, we want to ignore tracing enemies or main character
	GetHitResultUnderCursor(TraceChannel, false, CursorHit);

	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	if (IsValid(CursorHit.GetActor()) && CursorHit.GetActor()->Implements<UHighlightInterface>())
	{
		ThisActor = CursorHit.GetActor();
	}
	else
	{
		ThisActor = nullptr;
	}

	if (LastActor != ThisActor)
	{
		UnHighLightActor(LastActor);
		HighLightActor(ThisActor);
	}
}

void ASeptentPlayerController::HighLightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_HighlightActor(InActor);
	}
}

void ASeptentPlayerController::UnHighLightActor(AActor* InActor)
{
	if (IsValid(InActor) && InActor->Implements<UHighlightInterface>())
	{
		IHighlightInterface::Execute_UnHighlightActor(InActor);
	}
}

void ASeptentPlayerController::AbilityInputTagPressed(FGameplayTag InputTag)
{
	if (GetAbilitySystemComponent() && GetAbilitySystemComponent()->HasMatchingGameplayTag(FSeptentGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	// GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, FString::Printf(TEXT("Pressed %s"), *InputTag.ToString()));
	if (InputTag.MatchesTagExact(FSeptentGameplayTags::Get().InputTag_LMB))
	{
		if (IsValid(ThisActor))
		{
			TargetingStatus = ThisActor->Implements<UEnemyInterface>() ? ETargetingStatus::TargetingEnemy : ETargetingStatus::TargetingNonenemy;
		}
		else
		{
			TargetingStatus = ETargetingStatus::NotTargeting;
		}
		bAutoRunning = false;
	}
	if (GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->AbilityInputTagPressed(InputTag);
	}
}

void ASeptentPlayerController::AbilityInputTagReleased(FGameplayTag InputTag)
{
	if (GetAbilitySystemComponent() && GetAbilitySystemComponent()->HasMatchingGameplayTag(FSeptentGameplayTags::Get().Player_Block_InputReleased))
	{
		return;
	}
	
	if (!InputTag.MatchesTagExact(FSeptentGameplayTags::Get().InputTag_LMB))
	{
		if (GetAbilitySystemComponent())
		{
			GetAbilitySystemComponent()->AbilityInputTagReleased(InputTag);
		}
		return;
	}

	if (GetAbilitySystemComponent())
	{
		GetAbilitySystemComponent()->AbilityInputTagReleased(InputTag);
	}

	if (TargetingStatus != ETargetingStatus::TargetingEnemy && !bShiftPressed)
	{
		const APawn* ControlledPawn = GetPawn<APawn>();
		if (FollowTime <= ShortPressThreshold && ControlledPawn)
		{
			if (IsValid(ThisActor) && ThisActor->Implements<UHighlightInterface>())
			{
				IHighlightInterface::Execute_SetMoveToLocation(ThisActor, CachedDestination);
			}
			else if (GetAbilitySystemComponent() && !GetAbilitySystemComponent()->HasMatchingGameplayTag(FSeptentGameplayTags::Get().Player_Block_InputPressed))
			{
				UNiagaraFunctionLibrary::SpawnSystemAtLocation(this, ClickNiagaraSystem, CachedDestination);
			}
			
			if (UNavigationPath* NavPath = UNavigationSystemV1::FindPathToLocationSynchronously(this, ControlledPawn->GetActorLocation(), CachedDestination))
			{
				Spline->ClearSplinePoints();
				for (const FVector& PointLoc : NavPath->PathPoints)
				{
					Spline->AddSplinePoint(PointLoc, ESplineCoordinateSpace::World);
					// DrawDebugSphere(GetWorld(), PointLoc, 10.f, 10, FColor::Green, false, 5.f);
				}
				if (NavPath->PathPoints.IsEmpty())
				{
					CachedDestination = ControlledPawn->GetActorLocation();
				} else
				{
					CachedDestination = NavPath->PathPoints.Last();
				}
				bAutoRunning = true;
			}
		}
		FollowTime = 0.f;
		TargetingStatus = ETargetingStatus::NotTargeting;
	}
}

void ASeptentPlayerController::AbilityInputTagHeld(FGameplayTag InputTag)
{
	if (GetAbilitySystemComponent() && GetAbilitySystemComponent()->HasMatchingGameplayTag(FSeptentGameplayTags::Get().Player_Block_InputHeld))
	{
		return;
	}
	
	if (!InputTag.MatchesTagExact(FSeptentGameplayTags::Get().InputTag_LMB))
	{
		if (GetAbilitySystemComponent())
		{
			GetAbilitySystemComponent()->AbilityInputTagHeld(InputTag);
		}
		return;
	}

	if (TargetingStatus == ETargetingStatus::TargetingEnemy || bShiftPressed)
	{
		if (GetAbilitySystemComponent())
		{
			GetAbilitySystemComponent()->AbilityInputTagHeld(InputTag);
		}
	}
	else
	{
		FollowTime += GetWorld()->GetDeltaSeconds();
		
		if (CursorHit.bBlockingHit)
		{
			CachedDestination = CursorHit.ImpactPoint;
		}

		if (APawn* ControlledPawn = GetPawn<APawn>())
		{
			const FVector WorldDirection = (CachedDestination - ControlledPawn->GetActorLocation()).GetSafeNormal();
			ControlledPawn->AddMovementInput(WorldDirection, 1.f);
		}
	}
}

USeptentAbilitySystemComponent* ASeptentPlayerController::GetAbilitySystemComponent()
{
	if (SeptentAbilitySystemComponent == nullptr)
	{
		SeptentAbilitySystemComponent = Cast<USeptentAbilitySystemComponent>(UAbilitySystemBlueprintLibrary::GetAbilitySystemComponent(GetPawn<APawn>()));
	}
	return SeptentAbilitySystemComponent;
}

void ASeptentPlayerController::BeginPlay()
{
	Super::BeginPlay();
	check(SeptentInputMappingContext);

	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());
	if(Subsystem)
	{
		Subsystem->AddMappingContext(SeptentInputMappingContext, 0);
	}
	
	bShowMouseCursor = true;
	DefaultMouseCursor = EMouseCursor::Default;

	FInputModeGameAndUI InputModeData;
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);
	InputModeData.SetHideCursorDuringCapture(false);
	SetInputMode(InputModeData);
}

void ASeptentPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	USeptentInputComponent* SeptentInputComponent = CastChecked<USeptentInputComponent>(InputComponent);

	SeptentInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASeptentPlayerController::Move);
	SeptentInputComponent->BindAction(ShiftAction, ETriggerEvent::Started, this, &ASeptentPlayerController::ShiftPressed);
	SeptentInputComponent->BindAction(ShiftAction, ETriggerEvent::Completed, this, &ASeptentPlayerController::ShiftReleased);
	SeptentInputComponent->BindAbilityActions(InputConfig, this, &ThisClass::AbilityInputTagPressed, &ThisClass::AbilityInputTagReleased, &ThisClass::AbilityInputTagHeld);
}

void ASeptentPlayerController::Move(const FInputActionValue& InputActionValue) 
{
	if (GetAbilitySystemComponent() && GetAbilitySystemComponent()->HasMatchingGameplayTag(FSeptentGameplayTags::Get().Player_Block_InputPressed))
	{
		return;
	}
	
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();
	const FRotator Rotation = GetControlRotation();
	const FRotator YawRotation(0, Rotation.Yaw, 0);

	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

	if (APawn* ControlledPawn = GetPawn<APawn>())
	{
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}


