// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsWeaponTuning.h"

bool UGoombanicsWeaponTuningDataAsset::TryGetTuning(EGoombanicsWeaponId WeaponId, FGoombanicsWeaponTuning& OutTuning) const
{
	for (const FGoombanicsWeaponTuning& Entry : Weapons)
	{
		if (Entry.WeaponId == WeaponId)
		{
			OutTuning = Entry;
			return true;
		}
	}

	return false;
}
