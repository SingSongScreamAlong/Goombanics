// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerState.h"
#include "Goombanics/Core/GoombanicsTypes.h"
#include "GoombanicsPlayerState.generated.h"

// -----------------------------------------------------------------------------
// AGoombanicsPlayerState
//
// Responsibilities:
// - Own and replicate player meta-state (Role) and scoring data.
// - Remains authoritative on server; clients read replicated data for HUD/scoreboard.
//
// Replication intent (Phase 2 Windows validation):
// - Role replicated so UI and gameplay can branch without hardcoded player indices.
// - Score data replicated for scalable scoreboard (supports >4 online players later).
//
// TODO(OnlineScaling): Move score aggregation to server-only updates with explicit RPCs if needed.
// TODO(PlayerControlledKaiju): When a player is assigned Role=Kaiju, GameMode will possess the Kaiju pawn.
// -----------------------------------------------------------------------------

UCLASS()
class GOOMBANICS_API AGoombanicsPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	AGoombanicsPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	UFUNCTION(BlueprintPure, Category = "Goombanics|Role")
	EGoombanicsRole GetRole() const { return Role; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Role")
	void SetRole(EGoombanicsRole NewRole);

	// Deprecated wrappers: prefer GetRole/SetRole.
	UFUNCTION(BlueprintPure, Category = "Goombanics|Role", meta = (DeprecatedFunction, DeprecationMessage = "Use GetRole()"))
	EGoombanicsPlayerRole GetPlayerRole() const
	{
		switch (Role)
		{
		case EGoombanicsRole::Kaiju:
			return EGoombanicsPlayerRole::Kaiju;
		case EGoombanicsRole::Spectator:
			return EGoombanicsPlayerRole::Spectator;
		case EGoombanicsRole::Human:
		default:
			return EGoombanicsPlayerRole::Human;
		}
	}

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Role", meta = (DeprecatedFunction, DeprecationMessage = "Use SetRole()"))
	void SetPlayerRole(EGoombanicsPlayerRole NewRole)
	{
		switch (NewRole)
		{
		case EGoombanicsPlayerRole::Kaiju:
			SetRole(EGoombanicsRole::Kaiju);
			break;
		case EGoombanicsPlayerRole::Spectator:
			SetRole(EGoombanicsRole::Spectator);
			break;
		case EGoombanicsPlayerRole::Human:
		default:
			SetRole(EGoombanicsRole::Human);
			break;
		}
	}

	UFUNCTION(BlueprintPure, Category = "Goombanics|Score")
	const FGoombanicsPlayerScoreData& GetScoreData() const { return ScoreData; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Score")
	float GetTotalScore() const { return ScoreData.TotalScore; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Score")
	float GetKaijuDamageDealt() const { return ScoreData.KaijuDamageDealt; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Score")
	float GetCollateralDamageScore() const { return ScoreData.CollateralDamageScore; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Score")
	int32 GetWeakPointsDestroyed() const { return ScoreData.WeakPointsDestroyed; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Score")
	int32 GetDeaths() const { return ScoreData.Deaths; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Score")
	int32 GetFinalBlowCount() const { return ScoreData.FinalBlowCount; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Score")
	void AddKaijuDamage(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Score")
	void AddCollateralDamage(float Damage);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Score")
	void IncrementWeakPointsDestroyed();

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Score")
	void IncrementDeaths();

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Score")
	void IncrementFinalBlowCount();

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Score")
	void CalculateFinalScore();

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Score")
	void ResetScore();

protected:
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Goombanics|Role")
	EGoombanicsRole Role = EGoombanicsRole::Human;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Goombanics|Score")
	FGoombanicsPlayerScoreData ScoreData;
};
