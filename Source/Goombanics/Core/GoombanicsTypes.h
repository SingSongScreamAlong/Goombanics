// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GoombanicsTypes.generated.h"

// -----------------------------------------------------------------------------
// GOOMBANICS: COLLATERAL DAMAGE - Core shared types
//
// NOTE (WRITE-ONLY): This file is intended to be safe for UE 5.5.x builds later
// on Windows. Keep it lightweight and dependency-free.
//
// TODO(Phase2-Windows): Validate replication + UI bindings in PIE with 1-4 local
// players and on a listen server.
// -----------------------------------------------------------------------------

UENUM(BlueprintType)
enum class EGoombanicsMatchPhase : uint8
{
	None		UMETA(DisplayName = "None"),
	Warmup		UMETA(DisplayName = "Warmup"),
	InProgress	UMETA(DisplayName = "In Progress"),
	PostRound	UMETA(DisplayName = "Post Round")
};

// Role model requirement:
// - PlayerState owns the role (replicated)
// - GameMode assigns roles at match start / join
// - Kaiju role is reserved for future PvPvE (player-controllable Kaiju)
UENUM(BlueprintType)
enum class EGoombanicsRole : uint8
{
	Human	UMETA(DisplayName = "Human"),
	Kaiju	UMETA(DisplayName = "Kaiju"),
	Spectator	UMETA(DisplayName = "Spectator")
};

UENUM(BlueprintType)
// Deprecated: prefer EGoombanicsRole.
enum class EGoombanicsPlayerRole : uint8
{
	Human		UMETA(DisplayName = "Human"),
	Kaiju		UMETA(DisplayName = "Kaiju"),
	Spectator	UMETA(DisplayName = "Spectator")
};

UENUM(BlueprintType)
enum class EGoombanicsWeakPointType : uint8
{
	None		UMETA(DisplayName = "None"),
	LeftLeg		UMETA(DisplayName = "Left Leg"),
	RightLeg	UMETA(DisplayName = "Right Leg"),
	Head		UMETA(DisplayName = "Head")
};

UENUM(BlueprintType)
enum class EGoombanicsMatchEndReason : uint8
{
	None				UMETA(DisplayName = "None"),
	KaijuDefeated		UMETA(DisplayName = "Kaiju Defeated"),
	CityDestroyed		UMETA(DisplayName = "City Destroyed"),
	TimerExpired		UMETA(DisplayName = "Timer Expired")
};

USTRUCT(BlueprintType)
struct FGoombanicsScoreboardEntry
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	FString PlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	int32 PlayerId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	float TotalScore = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	float KaijuDamageDealt = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	float CollateralDamageScore = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	int32 WeakPointsDestroyed = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	int32 Deaths = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Scoreboard")
	bool bIsLocalPlayer = false;
};

UENUM(BlueprintType)
enum class EGoombanicsAwardType : uint8
{
	MostKaijuDamage		UMETA(DisplayName = "Most Kaiju Damage"),
	MostCollateralDamage	UMETA(DisplayName = "Most Collateral Damage"),
	MostDeaths			UMETA(DisplayName = "Most Deaths"),
	FinalBlow			UMETA(DisplayName = "Final Blow"),
	WorstDayEver		UMETA(DisplayName = "Worst Day Ever")
};

USTRUCT(BlueprintType)
struct FGoombanicsAwardResult
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Awards")
	EGoombanicsAwardType AwardType = EGoombanicsAwardType::MostKaijuDamage;

	UPROPERTY(BlueprintReadOnly, Category = "Awards")
	FString WinnerPlayerName;

	UPROPERTY(BlueprintReadOnly, Category = "Awards")
	int32 WinnerPlayerId = -1;

	UPROPERTY(BlueprintReadOnly, Category = "Awards")
	float Value = 0.0f;
};

USTRUCT(BlueprintType)
struct FGoombanicsEndOfRoundAwards
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Awards")
	TArray<FGoombanicsAwardResult> Awards;
};

USTRUCT(BlueprintType)
struct FGoombanicsWeakPointState
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weak Point")
	EGoombanicsWeakPointType WeakPointType = EGoombanicsWeakPointType::None;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weak Point")
	float CurrentHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weak Point")
	float MaxHealth = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Weak Point")
	bool bIsDestroyed = false;

	float GetHealthPercent() const
	{
		return MaxHealth > 0.0f ? FMath::Clamp(CurrentHealth / MaxHealth, 0.0f, 1.0f) : 0.0f;
	}
};

USTRUCT(BlueprintType)
struct FGoombanicsScoreWeights
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	float KaijuDamageWeight = 1.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	float CollateralDamageWeight = 0.5f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	float WeakPointDestroyedBonus = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	float FinalBlowBonus = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Scoring")
	float DeathPenalty = 100.0f;
};

USTRUCT(BlueprintType)
struct FGoombanicsPlayerScoreData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	float KaijuDamageDealt = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	float CollateralDamageScore = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 WeakPointsDestroyed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 Deaths = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	int32 FinalBlowCount = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Score")
	float TotalScore = 0.0f;

	void CalculateTotalScore(const FGoombanicsScoreWeights& Weights)
	{
		TotalScore = (KaijuDamageDealt * Weights.KaijuDamageWeight)
			+ (CollateralDamageScore * Weights.CollateralDamageWeight)
			+ (WeakPointsDestroyed * Weights.WeakPointDestroyedBonus)
			+ (FinalBlowCount * Weights.FinalBlowBonus)
			- (Deaths * Weights.DeathPenalty);
		TotalScore = FMath::Max(0.0f, TotalScore);
	}
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchPhaseChanged, EGoombanicsMatchPhase, NewPhase);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnDestructionPercentChanged, float, NewPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKaijuHealthChanged, float, NewHealthPercent);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnWeakPointDestroyed, EGoombanicsWeakPointType, WeakPointType, APlayerState*, Destroyer);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnKaijuStaggered, bool, bIsStaggered);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnMatchEnded, EGoombanicsMatchEndReason, EndReason);
