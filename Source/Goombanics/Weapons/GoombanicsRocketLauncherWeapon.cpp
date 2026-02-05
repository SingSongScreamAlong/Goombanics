// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsRocketLauncherWeapon.h"

AGoombanicsRocketLauncherWeapon::AGoombanicsRocketLauncherWeapon()
{
	WeaponId = EGoombanicsWeaponId::RocketLauncher;

	RuntimeTuning.WeaponId = WeaponId;
	RuntimeTuning.FireRate = 1.0f;
	RuntimeTuning.Damage = 120.0f;
	RuntimeTuning.SplashRadius = 450.0f;
	RuntimeTuning.CameraShakeIntensity = 1.0f;
	RuntimeTuning.RagdollImpulseStrength = 6000.0f;
	RuntimeTuning.DestructionBias = 0.75f;
}
