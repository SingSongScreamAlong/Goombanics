// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GoombanicsBaseWeapon.h"
#include "GoombanicsRifleWeapon.generated.h"

UCLASS(Blueprintable)
class GOOMBANICS_API AGoombanicsRifleWeapon : public AGoombanicsBaseWeapon
{
	GENERATED_BODY()

public:
	AGoombanicsRifleWeapon();

	// FEEL INTENT:
	// - Constant pressure.
	// - Clear, readable hit feedback.
	// - Low chaos, high consistency.
};
