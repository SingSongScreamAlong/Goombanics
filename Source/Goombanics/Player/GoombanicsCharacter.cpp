// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsCharacter.h"
#include "GoombanicsPlayerState.h"
#include "Goombanics/Weapons/GoombanicsWeaponComponent.h"
#include "Goombanics/Core/GoombanicsGameMode.h"
#include "Goombanics/Goombanics.h"
#include "EnhancedInputComponent.h"
#include "EnhancedInputSubsystems.h"
#include "InputActionValue.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/CapsuleComponent.h"

AGoombanicsCharacter::AGoombanicsCharacter()
{
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	GetCharacterMovement()->bOrientRotationToMovement = false;
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f);
	GetCharacterMovement()->JumpZVelocity = 600.0f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.0f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.0f;

	WeaponComponent = CreateDefaultSubobject<UGoombanicsWeaponComponent>(TEXT("WeaponComponent"));

	CurrentHealth = MaxHealth;
}

void AGoombanicsCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (APlayerController* PC = Cast<APlayerController>(Controller))
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer()))
		{
			if (DefaultMappingContext)
			{
				Subsystem->AddMappingContext(DefaultMappingContext, 0);
			}
		}
	}

	CurrentHealth = MaxHealth;
}

void AGoombanicsCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	UpdateDash(DeltaTime);

	if (DashCooldownRemaining > 0.0f)
	{
		DashCooldownRemaining -= DeltaTime;
	}
}

void AGoombanicsCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		if (MoveAction)
		{
			EnhancedInputComponent->BindAction(MoveAction, ETriggerEvent::Triggered, this, &AGoombanicsCharacter::Move);
		}
		if (LookAction)
		{
			EnhancedInputComponent->BindAction(LookAction, ETriggerEvent::Triggered, this, &AGoombanicsCharacter::Look);
		}
		if (JumpAction)
		{
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Started, this, &AGoombanicsCharacter::StartJump);
			EnhancedInputComponent->BindAction(JumpAction, ETriggerEvent::Completed, this, &AGoombanicsCharacter::StopJump);
		}
		if (FireAction)
		{
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Started, this, &AGoombanicsCharacter::StartFire);
			EnhancedInputComponent->BindAction(FireAction, ETriggerEvent::Completed, this, &AGoombanicsCharacter::StopFire);
		}
		if (DashAction)
		{
			EnhancedInputComponent->BindAction(DashAction, ETriggerEvent::Started, this, &AGoombanicsCharacter::StartDash);
		}
		if (SwitchWeaponAction)
		{
			EnhancedInputComponent->BindAction(SwitchWeaponAction, ETriggerEvent::Started, this, &AGoombanicsCharacter::SwitchWeapon);
		}
	}
}

float AGoombanicsCharacter::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	float ActualDamage = Super::TakeDamage(DamageAmount, DamageEvent, EventInstigator, DamageCauser);

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - ActualDamage);

	if (CurrentHealth <= 0.0f)
	{
		Die();
	}

	return ActualDamage;
}

void AGoombanicsCharacter::Die()
{
	if (AController* MyController = Controller)
	{
		if (AGoombanicsGameMode* GM = GetWorld()->GetAuthGameMode<AGoombanicsGameMode>())
		{
			GM->RequestPlayerRespawn(MyController);
		}
	}

	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
	GetCharacterMovement()->DisableMovement();

	UE_LOG(LogGoombanics, Log, TEXT("Player died"));
}

void AGoombanicsCharacter::Move(const FInputActionValue& Value)
{
	if (bIsDashing)
	{
		return;
	}

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

void AGoombanicsCharacter::Look(const FInputActionValue& Value)
{
	FVector2D LookAxisVector = Value.Get<FVector2D>();

	if (Controller != nullptr)
	{
		AddControllerYawInput(LookAxisVector.X);
		AddControllerPitchInput(LookAxisVector.Y);
	}
}

void AGoombanicsCharacter::StartJump()
{
	Jump();
}

void AGoombanicsCharacter::StopJump()
{
	StopJumping();
}

void AGoombanicsCharacter::StartFire()
{
	if (WeaponComponent)
	{
		WeaponComponent->StartFire();
	}
}

void AGoombanicsCharacter::StopFire()
{
	if (WeaponComponent)
	{
		WeaponComponent->StopFire();
	}
}

void AGoombanicsCharacter::SwitchWeapon()
{
	if (WeaponComponent)
	{
		WeaponComponent->SwitchToNextWeapon();
	}
}

void AGoombanicsCharacter::StartDash()
{
	if (bIsDashing || DashCooldownRemaining > 0.0f)
	{
		return;
	}

	FVector Velocity = GetCharacterMovement()->Velocity;
	Velocity.Z = 0.0f;

	if (Velocity.IsNearlyZero())
	{
		DashDirection = GetActorForwardVector();
	}
	else
	{
		DashDirection = Velocity.GetSafeNormal();
	}

	bIsDashing = true;
	DashTimeRemaining = DashDuration;
	DashCooldownRemaining = DashCooldown;

	GetCharacterMovement()->GroundFriction = 0.0f;
}

void AGoombanicsCharacter::UpdateDash(float DeltaTime)
{
	if (!bIsDashing)
	{
		return;
	}

	DashTimeRemaining -= DeltaTime;

	if (DashTimeRemaining <= 0.0f)
	{
		bIsDashing = false;
		GetCharacterMovement()->GroundFriction = 8.0f;
		return;
	}

	float DashSpeed = DashDistance / DashDuration;
	FVector DashVelocity = DashDirection * DashSpeed;
	DashVelocity.Z = GetCharacterMovement()->Velocity.Z;
	GetCharacterMovement()->Velocity = DashVelocity;
}
