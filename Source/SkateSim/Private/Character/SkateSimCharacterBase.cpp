// Fill out your copyright notice in the Description page of Project Settings.


#include "Character/SkateSimCharacterBase.h"
#include "Engine/LocalPlayer.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "GameFramework/Controller.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"

ASkateSimCharacterBase::ASkateSimCharacterBase()
{
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);
		
	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...	
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 400.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	GetCharacterMovement()->BrakingDecelerationFalling = 1500.0f;

	// Create a camera boom (pulls in towards the player if there is a collision)
	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(RootComponent);
	CameraBoom->TargetArmLength = 400.0f; // The camera follows at this distance behind the character	
	CameraBoom->bUsePawnControlRotation = true; // Rotate the arm based on the controller

	// Create a follow camera
	FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
	FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName); // Attach the camera to the end of the boom and let the boom adjust to match the controller orientation
	FollowCamera->bUsePawnControlRotation = false; // Camera does not rotate relative to arm

	CurrentVelocity = FVector::ZeroVector;
	MaxSpeed = 400.0f;
    AccelerationRate = 2000.0f;
	DecelerationRate = 0.5f;
	PushImpulseStrength = 4000.0f; 
	SlowdownFactor = 0.f;

	Skate = CreateDefaultSubobject<UStaticMeshComponent>("Skate Mesh");
	Skate->SetupAttachment(GetMesh(), FName("SkateSocket"));

	JumpedOverCount = 0;
	JumpFailedCount = 0;
}

void ASkateSimCharacterBase::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PlayerController = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PlayerController->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}	
}

void ASkateSimCharacterBase::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    // Set Correct Skate Rotation To Align With Animations
    if(Skate)
    {
        float CapsuleYaw = GetCapsuleComponent()->GetComponentRotation().Yaw;
        FRotator NewRotation = FRotator(0.f, CapsuleYaw + 90.f, 0.f);
        Skate->SetWorldRotation(NewRotation);
    }

    // Decrease Velocity
    if (!CurrentVelocity.IsZero())
    {
        AddMovementInput(CurrentVelocity.GetSafeNormal(), CurrentVelocity.Size() * DeltaTime);
        CurrentVelocity = FMath::VInterpTo(CurrentVelocity, FVector::ZeroVector, DeltaTime, DecelerationRate);
        
        // Update MaxWalkSpeed to match current speed
        GetCharacterMovement()->MaxWalkSpeed = CurrentVelocity.Size();
    }
}

void ASkateSimCharacterBase::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Set up action bindings
	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		
		// Jumping
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &ACharacter::Jump);
		EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &ACharacter::StopJumping);

		// Moving
		EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &ASkateSimCharacterBase::Move);

		// Looking
		EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &ASkateSimCharacterBase::Look);

		// Pushing
		EnhancedInputComponent->BindAction(PushAction, ETriggerEvent::Started, this, &ASkateSimCharacterBase::Push);
		EnhancedInputComponent->BindAction(PushAction, ETriggerEvent::Completed, this, &ASkateSimCharacterBase::Push);

		// Slowdowning
		EnhancedInputComponent->BindAction(SlowdownAction, ETriggerEvent::Started, this, &ASkateSimCharacterBase::Slowdown);
		EnhancedInputComponent->BindAction(SlowdownAction, ETriggerEvent::Completed, this, &ASkateSimCharacterBase::Slowdown);
	}
}

void ASkateSimCharacterBase::Move(const FInputActionValue& Value)
{
    // input is a Vector2D
    FVector2D MovementVector = Value.Get<FVector2D>();

    if (Controller != nullptr)
    {
        // find out which way is forward
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        // get forward vector
        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
    
        // get right vector 
        const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

        // apply movement
        FVector Impulse = (ForwardDirection * MovementVector.Y + RightDirection * MovementVector.X) * AccelerationRate * GetWorld()->GetDeltaSeconds();
        CurrentVelocity = FVector(FMath::Clamp(CurrentVelocity.X + Impulse.X, -MaxSpeed, MaxSpeed),
        FMath::Clamp(CurrentVelocity.Y + Impulse.Y, -MaxSpeed, MaxSpeed), CurrentVelocity.Z);

        // Update MaxWalkSpeed based on CurrentVelocity
        GetCharacterMovement()->MaxWalkSpeed = CurrentVelocity.Size();
    }
}

void ASkateSimCharacterBase::Look(const FInputActionValue& Value)
{
	// input is a Vector2D
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		// add yaw and pitch input to controller
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void ASkateSimCharacterBase::Push(const FInputActionValue& Value)
{
    if (Controller != nullptr && !bPush)
    {
        const FRotator Rotation = Controller->GetControlRotation();
        const FRotator YawRotation(0, Rotation.Yaw, 0);

        const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);

        FVector Impulse = ForwardDirection * PushImpulseStrength;
        CurrentVelocity += Impulse;

        // Ensure MaxWalkSpeed is updated to allow for higher speeds
        GetCharacterMovement()->MaxWalkSpeed = FMath::Clamp(CurrentVelocity.Size(), MaxSpeed, 800.0f);        
    }

    bPush = Value.Get<bool>();
}

void ASkateSimCharacterBase::Slowdown(const FInputActionValue& Value)
{
    if (Controller != nullptr && !bSlowDown)
    {
        CurrentVelocity *= SlowdownFactor;		
    }

	bSlowDown = Value.Get<bool>();
}

int32 ASkateSimCharacterBase::GetJumpFailedCount()
{
	return JumpFailedCount;
}

int32 ASkateSimCharacterBase::GetJumpedOverCount()
{
	return JumpedOverCount;
}

void ASkateSimCharacterBase::IncrementJumpFailed()
{
	JumpFailedCount++;
}

void ASkateSimCharacterBase::IncrementJumpedOver()
{
	JumpedOverCount++;
}
