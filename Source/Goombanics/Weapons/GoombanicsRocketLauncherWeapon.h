// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GoombanicsBaseWeapon.h"
#include "GoombanicsRocketLauncherWeapon.generated.h"

UCLASS(Blueprintable)
class GOOMBANICS_API AGoombanicsRocketLauncherWeapon : public AGoombanicsBaseWeapon
{
	GENERATED_BODY()

public:
	AGoombanicsRocketLauncherWeapon();

	// FEEL INTENT:
	// - Chaos, knockback, comedy.
	// - Big readable explosions.
	// - Strong environmental interaction (without requiring Chaos).
};
