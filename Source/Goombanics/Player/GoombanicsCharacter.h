// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "GoombanicsCharacter.generated.h"

class UInputMappingContext;
class UInputAction;
class UGoombanicsWeaponComponent;
struct FInputActionValue;

UCLASS()
class GOOMBANICS_API AGoombanicsCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGoombanicsCharacter();

	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;
	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual float TakeDamage(float DamageAmount, struct FDamageEvent const& DamageEvent, class AController* EventInstigator, AActor* DamageCauser) override;

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Combat")
	void Die();

	UFUNCTION(BlueprintPure, Category = "Goombanics|Combat")
	bool IsAlive() const { return CurrentHealth > 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Combat")
	float GetHealthPercent() const { return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Weapons")
	UGoombanicsWeaponComponent* GetWeaponComponent() const { return WeaponComponent; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Movement")
	void StartDash();

	UFUNCTION(BlueprintPure, Category = "Goombanics|Movement")
	bool IsDashing() const { return bIsDashing; }

protected:
	void Move(const FInputActionValue& Value);
	void Look(const FInputActionValue& Value);
	void StartJump();
	void StopJump();
	void StartFire();
	void StopFire();
	void SwitchWeapon();
	void UpdateDash(float DeltaTime);

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Goombanics|Input")
	TObjectPtr<UInputMappingContext> DefaultMappingContext;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Goombanics|Input")
	TObjectPtr<UInputAction> MoveAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Goombanics|Input")
	TObjectPtr<UInputAction> LookAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Goombanics|Input")
	TObjectPtr<UInputAction> JumpAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Goombanics|Input")
	TObjectPtr<UInputAction> FireAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Goombanics|Input")
	TObjectPtr<UInputAction> DashAction;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Goombanics|Input")
	TObjectPtr<UInputAction> SwitchWeaponAction;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goombanics|Weapons")
	TObjectPtr<UGoombanicsWeaponComponent> WeaponComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Combat")
	float MaxHealth = 100.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Goombanics|Combat")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Movement")
	float DashDistance = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Movement")
	float DashDuration = 0.2f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Movement")
	float DashCooldown = 1.0f;

	bool bIsDashing = false;
	float DashTimeRemaining = 0.0f;
	float DashCooldownRemaining = 0.0f;
	FVector DashDirection = FVector::ZeroVector;
};
