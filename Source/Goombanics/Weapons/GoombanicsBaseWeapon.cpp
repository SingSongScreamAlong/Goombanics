// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsBaseWeapon.h"

AGoombanicsBaseWeapon::AGoombanicsBaseWeapon()
{
	PrimaryActorTick.bCanEverTick = false;

	RuntimeTuning.WeaponId = WeaponId;
}

void AGoombanicsBaseWeapon::SetRuntimeTuning(const FGoombanicsWeaponTuning& InTuning)
{
	RuntimeTuning = InTuning;
	WeaponId = InTuning.WeaponId;
}
