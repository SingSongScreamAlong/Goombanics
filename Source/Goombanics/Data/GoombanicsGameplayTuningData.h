// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Goombanics/Core/GoombanicsTypes.h"
#include "GoombanicsGameplayTuningData.generated.h"

USTRUCT(BlueprintType)
struct FGoombanicsMatchDefaults
{
	GENERATED_BODY()

	// Intended to match GameMode defaults until we go fully data-driven.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goombanics|Match")
	float WarmupDuration = 5.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goombanics|Match")
	float MatchDuration = 180.0f;
};

USTRUCT(BlueprintType)
struct FGoombanicsRespawnTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goombanics|Respawn")
	float RespawnDelay = 4.0f;
};

USTRUCT(BlueprintType)
struct FGoombanicsKaijuTuning
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goombanics|Kaiju")
	float MaxHealth = 10000.0f;

	// Stagger is triggered by destroying both leg weak points.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goombanics|Kaiju")
	float StaggerDuration = 6.0f;

	// Bonus window (typically overlaps stagger). Multiplies head damage.
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goombanics|Kaiju")
	float HeadBonusDamageMultiplier = 2.0f;
};

USTRUCT(BlueprintType)
struct FGoombanicsScoreMultipliers
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goombanics|Score")
	float WeakPointDuringStaggerMultiplier = 1.25f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goombanics|Score")
	float CollateralChainMultiplier = 1.15f;
};

UCLASS(BlueprintType)
class GOOMBANICS_API UGoombanicsMatchDefaultsDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Match")
	FGoombanicsMatchDefaults Match;
};

UCLASS(BlueprintType)
class GOOMBANICS_API UGoombanicsRespawnTuningDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Respawn")
	FGoombanicsRespawnTuning Respawn;
};

UCLASS(BlueprintType)
class GOOMBANICS_API UGoombanicsKaijuTuningDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Kaiju")
	FGoombanicsKaijuTuning Kaiju;
};

UCLASS(BlueprintType)
class GOOMBANICS_API UGoombanicsScoreMultipliersDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Score")
	FGoombanicsScoreMultipliers Multipliers;
};

// TODO(Phase2-Windows):
// - Create DataAssets under /Content/Data/ and hook GameMode/GameState/Kaiju to read them.
// - Validate these values in PIE (1–4 local players) and iterate for feel.
