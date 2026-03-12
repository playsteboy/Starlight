// Fill out your copyright notice in the Description page of Project Settings.


#include "MainCharacter.h"
#include "MyGridManager.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "GameFramework/PlayerController.h"
#include "InputAction.h"
#include "InputMappingContext.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/CharacterMovementComponent.h"
AMainCharacter::AMainCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	ConstructorHelpers::FObjectFinder<UInputMappingContext> Move_IMC_OBJ(TEXT("/Game/Starlight/IMC/Move_IMC.Move_IMC"));
	if (Move_IMC_OBJ.Succeeded())
	{
		Move_IMC = Move_IMC_OBJ.Object;
	}
	ConstructorHelpers::FObjectFinder<UInputAction> Move_IA_OBJ(TEXT("/Game/Starlight/IMC/Input/Move_IA"));
	if (Move_IA_OBJ.Succeeded())
	{
		Move_IA = Move_IA_OBJ.Object;

	}
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	bUseControllerRotationYaw = false;
	bUseControllerRotationPitch = false;
	bUseControllerRotationRoll = false;
	GetCharacterMovement()->bOrientRotationToMovement = true;

	Health = 5;
	Score = 0;
	LightShardCount = 0;
	GetCharacterMovement()->MaxWalkSpeed = 1200.f;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 720.0f, 0.0f);

}

void AMainCharacter::BeginPlay()
{
	Super::BeginPlay();
	GridManager = Cast<AMyGridManager>(
		UGameplayStatics::GetActorOfClass(GetWorld(), AMyGridManager::StaticClass())
	);
	if (!GridManager)
	{
		UE_LOG(LogTemp, Warning, TEXT("GridManager not found!"));
	}
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			Subsystem->AddMappingContext(Move_IMC, 0);
		}
	}
}

// Called to bind functionality to input
void AMainCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	if (UEnhancedInputComponent* EnhancedInputComponent = CastChecked<UEnhancedInputComponent>(PlayerInputComponent))
	{
		EnhancedInputComponent->BindAction(Move_IA, ETriggerEvent::Triggered, this, &AMainCharacter::Move);
	}

}

void AMainCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}


void AMainCharacter::Move(const FInputActionValue& Value)
{
	FVector2D MovementVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		const FVector ForwardDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		const FVector RightDirection = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);

		AddMovementInput(ForwardDirection, MovementVector.Y);
		AddMovementInput(RightDirection, MovementVector.X);
	}
}

void AMainCharacter::SetScore(float Amount)
{
	Score = Amount;
}

void AMainCharacter::SetHealth(float Amount)
{
	Health = Amount;
}

int32 AMainCharacter::GetHealth() const
{
	return Health;
}

int32 AMainCharacter::GetScore() const
{
	return Score;
}

int32 AMainCharacter::GetLightShardCount() const
{
	return LightShardCount;
}

void AMainCharacter::SetLightShardCount(int32 Amount)
{
	LightShardCount = Amount;
}