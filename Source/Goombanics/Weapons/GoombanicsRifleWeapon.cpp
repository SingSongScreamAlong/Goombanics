// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsRifleWeapon.h"

AGoombanicsRifleWeapon::AGoombanicsRifleWeapon()
{
	WeaponId = EGoombanicsWeaponId::Rifle;

	RuntimeTuning.WeaponId = WeaponId;
	RuntimeTuning.FireRate = 12.0f;
	RuntimeTuning.Damage = 12.0f;
	RuntimeTuning.SplashRadius = 0.0f;
	RuntimeTuning.CameraShakeIntensity = 0.2f;
	RuntimeTuning.RagdollImpulseStrength = 900.0f;
	RuntimeTuning.DestructionBias = 0.15f;
}
