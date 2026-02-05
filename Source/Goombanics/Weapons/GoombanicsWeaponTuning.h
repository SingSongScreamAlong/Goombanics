// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GoombanicsWeaponTuning.generated.h"

UENUM(BlueprintType)
enum class EGoombanicsWeaponId : uint8
{
	Rifle		UMETA(DisplayName = "Rifle"),
	RocketLauncher	UMETA(DisplayName = "Rocket Launcher")
};

USTRUCT(BlueprintType)
struct FGoombanicsWeaponTuning
{
	GENERATED_BODY()

	// FEEL INTENT:
	// - Rifle: constant pressure, readable damage, mild recoil.
	// - Rocket: chaos, knockback, comedy, huge feedback.

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Tuning")
	EGoombanicsWeaponId WeaponId = EGoombanicsWeaponId::Rifle;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Tuning")
	float FireRate = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Tuning")
	float Damage = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Tuning")
	float SplashRadius = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Feedback")
	float CameraShakeIntensity = 0.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Feedback")
	float RagdollImpulseStrength = 1500.0f;

	// 0..1 bias hint for scoring/UI:
	// - 0 => favors Kaiju DPS
	// - 1 => favors Environment/Collateral
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weapon|Scoring")
	float DestructionBias = 0.25f;
};

UCLASS(BlueprintType)
class GOOMBANICS_API UGoombanicsWeaponTuningDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	// Data-driven tuning: create concrete assets in Phase 2 (Windows) and reference them
	// from weapon Blueprints or the WeaponComponent.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Weapon|Tuning")
	TArray<FGoombanicsWeaponTuning> Weapons;

	UFUNCTION(BlueprintPure, Category = "Weapon|Tuning")
	bool TryGetTuning(EGoombanicsWeaponId WeaponId, FGoombanicsWeaponTuning& OutTuning) const;
};
