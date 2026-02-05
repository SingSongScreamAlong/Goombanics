// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GoombanicsWeaponComponent.generated.h"

class UGoombanicsWeaponData;
class AGoombanicsProjectile;
class UGoombanicsWeaponTuningDataAsset;

UENUM(BlueprintType)
enum class EGoombanicsFireMode : uint8
{
	Hitscan		UMETA(DisplayName = "Hitscan"),
	Projectile	UMETA(DisplayName = "Projectile")
};

USTRUCT(BlueprintType)
struct FGoombanicsWeaponStats
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	FName WeaponName = NAME_None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goombanics|Weapon")
	EGoombanicsFireMode FireMode = EGoombanicsFireMode::Hitscan;

	// Feel/tuning (data-driven):
	// Assign a DataAsset in Blueprint later (Phase 2 Windows) to tune weapons without code changes.
	// This component remains the modular firing orchestrator for both offline split-screen and future online scaling.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Weapon|Tuning")
	TObjectPtr<UGoombanicsWeaponTuningDataAsset> WeaponTuningData = nullptr;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Damage = 25.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float FireRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float Range = 10000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float SplashRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float SplashDamage = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float ProjectileSpeed = 3000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	int32 AmmoCapacity = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	float ReloadTime = 2.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon")
	TSubclassOf<AGoombanicsProjectile> ProjectileClass;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeaponFired, int32, WeaponIndex, int32, AmmoRemaining);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnWeaponSwitched, int32, NewWeaponIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnReloadStarted, float, ReloadTime);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnReloadFinished);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GOOMBANICS_API UGoombanicsWeaponComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UGoombanicsWeaponComponent();

	virtual void BeginPlay() override;
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Weapons")
	void StartFire();

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Weapons")
	void StopFire();

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Weapons")
	void SwitchToNextWeapon();

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Weapons")
	void SwitchToWeapon(int32 WeaponIndex);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Weapons")
	void StartReload();

	UFUNCTION(BlueprintPure, Category = "Goombanics|Weapons")
	int32 GetCurrentWeaponIndex() const { return CurrentWeaponIndex; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Weapons")
	const FGoombanicsWeaponStats& GetCurrentWeaponStats() const;

	UFUNCTION(BlueprintPure, Category = "Goombanics|Weapons")
	int32 GetCurrentAmmo() const { return CurrentAmmo; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Weapons")
	bool IsReloading() const { return bIsReloading; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Weapons")
	bool IsFiring() const { return bWantsToFire; }

	UPROPERTY(BlueprintAssignable, Category = "Goombanics|Weapons|Events")
	FOnWeaponFired OnWeaponFired;

	UPROPERTY(BlueprintAssignable, Category = "Goombanics|Weapons|Events")
	FOnWeaponSwitched OnWeaponSwitched;

	UPROPERTY(BlueprintAssignable, Category = "Goombanics|Weapons|Events")
	FOnReloadStarted OnReloadStarted;

	UPROPERTY(BlueprintAssignable, Category = "Goombanics|Weapons|Events")
	FOnReloadFinished OnReloadFinished;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Weapons")
	TArray<FGoombanicsWeaponStats> Weapons;

protected:
	virtual void Fire();
	virtual void FireHitscan();
	virtual void FireProjectile();
	virtual void ProcessHit(const FHitResult& HitResult, float Damage);
	virtual void ApplySplashDamage(const FVector& Location, float Radius, float Damage);
	virtual FVector GetMuzzleLocation() const;
	virtual FVector GetAimDirection() const;

	UPROPERTY(BlueprintReadOnly, Category = "Goombanics|Weapons")
	int32 CurrentWeaponIndex = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Goombanics|Weapons")
	int32 CurrentAmmo = 0;

	bool bWantsToFire = false;
	bool bIsReloading = false;
	float FireCooldown = 0.0f;
	float ReloadTimeRemaining = 0.0f;

	static FGoombanicsWeaponStats EmptyWeaponStats;
};
