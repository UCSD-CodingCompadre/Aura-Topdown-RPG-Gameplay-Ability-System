// Fill out your copyright notice in the Description page of Project Settings.


#include "Player/AuraPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "EnhancedInputComponent.h"
#include "Interaction/EnemyInterface.h"

AAuraPlayerController::AAuraPlayerController()
{

    // Set replicates to true
	bReplicates = true;
}

void AAuraPlayerController::PlayerTick(float DeltaTime)
{
	Super::PlayerTick(DeltaTime);

	CursorTrace();
}

void AAuraPlayerController::CursorTrace()
{
	FHitResult CursorHit;
	GetHitResultUnderCursor(ECC_Visibility, false, CursorHit);
	if (!CursorHit.bBlockingHit) return;

	LastActor = ThisActor;
	ThisActor = Cast<IEnemyInterface>(CursorHit.GetActor());

	/**
	 * Line trace from cursor. There are several scenarios:
	 *  A. LastActor is null && ThisActor is null
	 *		- Do nothing
	 *	B. LastActor is null && ThisActor is valid
	 *		- Highlight ThisActor
	 *	C. LastActor is valid && ThisActor is null
	 *		- UnHighlight LastActor
	 *	D. Both actors are valid, but LastActor != ThisActor
	 *		- UnHighlight LastActor, and Highlight ThisActor
	 *	E. Both actors are valid, and are the same actor
	 *		- Do nothing
	 */

	if (LastActor == nullptr)
	{
		if (ThisActor != nullptr)
		{
			// Case B
			ThisActor->HighlightActor();
		}
		else
		{
			// Case A - both are null, do nothing
		}
	}
	else // LastActor is valid
	{
		if (ThisActor == nullptr)
		{
			// Case C
			LastActor->UnHighlightActor();
		}
		else // both actors are valid
		{
			if (LastActor != ThisActor)
			{
				// Case D
				LastActor->UnHighlightActor();
				ThisActor->HighlightActor();
			}
			else
			{
				// Case E - do nothing
			}
		}
	}
}

void AAuraPlayerController::BeginPlay()
{

    // Call the parent BeginPlay function
	Super::BeginPlay();

    // Check if we have a mapping context
	check(AuraContext);

    // Hold the enchanced input subsystem for the local player
	UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer());

    // Check if we have a enchaned input subsystem
	check(Subsystem);

    // Add the mapping context 
	Subsystem->AddMappingContext(AuraContext, 0);

    // Set the mouse cursor to show
	bShowMouseCursor = true;

    // Set the mouse cursor to the default mouse
	DefaultMouseCursor = EMouseCursor::Default;

    // Hold input mode data for the controller
	FInputModeGameAndUI InputModeData;

    // Set the mouse lock to not on the viewport
	InputModeData.SetLockMouseToViewportBehavior(EMouseLockMode::DoNotLock);

    // Set the cursor to not hide during capture
	InputModeData.SetHideCursorDuringCapture(false);

    // Set the input mode using the input mode data
	SetInputMode(InputModeData);
}

void AAuraPlayerController::SetupInputComponent()
{

    // Call the parent SetupInputComponent function
	Super::SetupInputComponent();

    // Hold the enchanced input component
	UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(InputComponent);

    // Bind the move action to the enchanced input component
	EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AAuraPlayerController::Move);
}

void AAuraPlayerController::Move(const FInputActionValue& InputActionValue)
{

    // Hold the 2D vector from the action input
	const FVector2D InputAxisVector = InputActionValue.Get<FVector2D>();

    // Hold the rotation from the controller
	const FRotator Rotation = GetControlRotation();

    // Hold the yaw rottation only from the controller
	const FRotator YawRotation(0.f, Rotation.Yaw, 0.f);

    // Hold the forward direction from the rotation matrix
	const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

    // Hold the right direction from the rotation matrix
	const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

    // Check the controller is controlling a pawn
	if (APawn* ControlledPawn = GetPawn<APawn>())
	{

        // Add the movement input for the forward direction
		ControlledPawn->AddMovementInput(ForwardDirection, InputAxisVector.Y);

        // Add the movement input for the right direction
		ControlledPawn->AddMovementInput(RightDirection, InputAxisVector.X);
	}
}