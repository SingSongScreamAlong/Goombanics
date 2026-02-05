// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoombanicsWeaponTuning.h"
#include "GoombanicsBaseWeapon.generated.h"

class UGoombanicsWeaponComponent;

UCLASS(Abstract, Blueprintable)
class GOOMBANICS_API AGoombanicsBaseWeapon : public AActor
{
	GENERATED_BODY()

public:
	AGoombanicsBaseWeapon();

	// Responsibilities:
	// - Holds identity + tuning reference for a weapon.
	// - Provides a stable object to attach meshes/FX/audio later (Blueprint).
	//
	// This is intentionally light: firing logic remains in UGoombanicsWeaponComponent
	// for modularity and future online scaling.
	
	UFUNCTION(BlueprintPure, Category = "Goombanics|Weapon")
	EGoombanicsWeaponId GetWeaponId() const { return WeaponId; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Weapon")
	const FGoombanicsWeaponTuning& GetRuntimeTuning() const { return RuntimeTuning; }

	// TODO(Phase2-Windows): Pull tuning from a data asset owned by GameState or a central tuning subsystem.
	UFUNCTION(BlueprintCallable, Category = "Goombanics|Weapon")
	void SetRuntimeTuning(const FGoombanicsWeaponTuning& InTuning);

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Goombanics|Weapon")
	EGoombanicsWeaponId WeaponId = EGoombanicsWeaponId::Rifle;

	// Runtime copy so we can override per-match (mutators, difficulty, etc.).
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Weapon")
	FGoombanicsWeaponTuning RuntimeTuning;
};
