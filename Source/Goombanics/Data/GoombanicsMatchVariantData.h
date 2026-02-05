// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "Goombanics/Core/GoombanicsTypes.h"
#include "GoombanicsMatchVariantData.generated.h"

UENUM(BlueprintType)
enum class EGoombanicsMatchVariantId : uint8
{
	ClassicPvE UMETA(DisplayName = "Classic PvE"),
	OpenKaijuPossession UMETA(DisplayName = "Open Kaiju Possession"),
};

USTRUCT(BlueprintType)
struct FGoombanicsOpenKaijuPossessionRules
{
	GENERATED_BODY()

	// If true, one player can possess/control the Kaiju pawn.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Variant")
	bool bEnableOpenKaijuPossession = true;

	// Intended default: 1. Kept data-driven for future variants.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Variant", meta = (ClampMin = "0", ClampMax = "1"))
	int32 NumKaijuPlayers = 1;

	// If true, possession can swap during the match (e.g. on death/timeout) via server-authoritative code.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Variant")
	bool bAllowMidMatchPossessionSwap = false;

	// If true, the Kaiju can start AI-controlled until a player takes control.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Variant")
	bool bAllowAIControlledKaijuWhenUnpossessed = true;

	// Optional: if a Kaiju player disconnects, allow server to hand control back to AI.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Variant")
	bool bFallbackToAIOnDisconnect = true;
};

UCLASS(BlueprintType)
class GOOMBANICS_API UGoombanicsMatchVariantDataAsset : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Variant")
	EGoombanicsMatchVariantId VariantId = EGoombanicsMatchVariantId::ClassicPvE;

	// Rules used when VariantId == OpenKaijuPossession.
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Goombanics|Variant")
	FGoombanicsOpenKaijuPossessionRules OpenKaijuPossession;

	// TODO(Phase2-Windows):
	// - Add a GameMode property for this asset (EditDefaultsOnly) and apply at InitGame/StartPlay.
	// - Server-authoritative selection of Kaiju controller and possession calls.
	// - Ensure UI adapts based on possessed pawn (human vs kaiju) for each local player.
};
