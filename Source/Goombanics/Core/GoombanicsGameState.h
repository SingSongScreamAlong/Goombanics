// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "GoombanicsTypes.h"
#include "GoombanicsGameState.generated.h"

UCLASS()
class GOOMBANICS_API AGoombanicsGameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	AGoombanicsGameState();

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

	// -----------------------------------------------------------------------------
	// UI Data Access (WRITE-ONLY scaffolding)
	//
	// Goal: UMG can bind to GameState/PlayerState getters without hard dependencies.
	// Phase 2 (Windows) will create widgets and bind to these functions.
	// -----------------------------------------------------------------------------

	UFUNCTION(BlueprintPure, Category = "Goombanics|Match")
	EGoombanicsMatchPhase GetMatchPhase() const { return MatchPhase; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Match")
	void SetMatchPhase(EGoombanicsMatchPhase NewPhase);

	UFUNCTION(BlueprintPure, Category = "Goombanics|Match")
	float GetTimeRemaining() const { return TimeRemaining; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Match")
	void SetTimeRemaining(float NewTime);

	UFUNCTION(BlueprintPure, Category = "Goombanics|Destruction")
	float GetDestructionPercent() const { return DestructionPercent; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Destruction")
	void SetDestructionPercent(float NewPercent);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Destruction")
	void AddDestructionValue(float Value, APlayerState* Instigator);

	UFUNCTION(BlueprintPure, Category = "Goombanics|Kaiju")
	float GetKaijuHealthPercent() const { return KaijuHealthPercent; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Kaiju")
	void SetKaijuHealthPercent(float NewPercent);

	UFUNCTION(BlueprintPure, Category = "Goombanics|Kaiju")
	bool IsTotalledAchieved() const { return bTotalledAchieved; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Kaiju")
	void SetTotalledAchieved(bool bAchieved);

	UFUNCTION(BlueprintPure, Category = "Goombanics|Kaiju")
	const TArray<FGoombanicsWeakPointState>& GetWeakPointStates() const { return WeakPointStates; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Kaiju")
	void SetWeakPointStates(const TArray<FGoombanicsWeakPointState>& NewStates);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Kaiju")
	void UpdateWeakPointState(EGoombanicsWeakPointType Type, float NewHealth, bool bDestroyed, APlayerState* Destroyer);

	UFUNCTION(BlueprintPure, Category = "Goombanics|Kaiju")
	bool IsKaijuStaggered() const { return bKaijuStaggered; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Kaiju")
	void SetKaijuStaggered(bool bStaggered);

	UFUNCTION(BlueprintPure, Category = "Goombanics|Scoring")
	const FGoombanicsScoreWeights& GetScoreWeights() const { return ScoreWeights; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Scoring")
	void SetScoreWeights(const FGoombanicsScoreWeights& NewWeights);

	UFUNCTION(BlueprintPure, Category = "Goombanics|Match")
	EGoombanicsMatchEndReason GetMatchEndReason() const { return MatchEndReason; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Match")
	void SetMatchEndReason(EGoombanicsMatchEndReason NewReason);

	UFUNCTION(BlueprintPure, Category = "Goombanics|UI")
	const FGoombanicsEndOfRoundAwards& GetEndOfRoundAwards() const { return EndOfRoundAwards; }

	// Computes award results from current PlayerStates.
	// Intended to be called by GameMode when transitioning to PostRound.
	UFUNCTION(BlueprintCallable, Category = "Goombanics|UI")
	void ComputeEndOfRoundAwards();

	// Returns the scoreboard sorted by TotalScore (desc).
	// - MaxEntries: Top-N cutoff.
	// - bIncludeAllLocals: ensures all local players appear even if not in Top-N.
	// - LocalPlayerController: optional context for setting bIsLocalPlayer on entries.
	UFUNCTION(BlueprintCallable, Category = "Goombanics|UI")
	TArray<FGoombanicsScoreboardEntry> GetSortedScoreboard(int32 MaxEntries, bool bIncludeAllLocals, APlayerController* LocalPlayerController) const;

	UFUNCTION(BlueprintCallable, Category = "Goombanics|UI")
	TArray<APlayerState*> GetAllPlayerStates() const;

	UFUNCTION(BlueprintPure, Category = "Goombanics|UI")
	bool IsLocalPlayerState(APlayerState* PlayerState, APlayerController* LocalPlayerController) const;

	UFUNCTION(BlueprintPure, Category = "Goombanics|Destruction")
	float GetTotalDestructionValue() const { return TotalDestructionValue; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Destruction")
	void SetTotalDestructionValue(float Value);

	UPROPERTY(BlueprintAssignable, Category = "Goombanics|Events")
	FOnMatchPhaseChanged OnMatchPhaseChanged;

	UPROPERTY(BlueprintAssignable, Category = "Goombanics|Events")
	FOnDestructionPercentChanged OnDestructionPercentChanged;

	UPROPERTY(BlueprintAssignable, Category = "Goombanics|Events")
	FOnKaijuHealthChanged OnKaijuHealthChanged;

	UPROPERTY(BlueprintAssignable, Category = "Goombanics|Events")
	FOnWeakPointDestroyed OnWeakPointDestroyed;

	UPROPERTY(BlueprintAssignable, Category = "Goombanics|Events")
	FOnKaijuStaggered OnKaijuStaggered;

	UPROPERTY(BlueprintAssignable, Category = "Goombanics|Events")
	FOnMatchEnded OnMatchEnded;

protected:
	UPROPERTY(ReplicatedUsing = OnRep_MatchPhase, BlueprintReadOnly, Category = "Goombanics|Match")
	EGoombanicsMatchPhase MatchPhase = EGoombanicsMatchPhase::None;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Goombanics|Match")
	float TimeRemaining = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_DestructionPercent, BlueprintReadOnly, Category = "Goombanics|Destruction")
	float DestructionPercent = 0.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Goombanics|Destruction")
	float TotalDestructionValue = 10000.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Goombanics|Destruction")
	float CurrentDestructionValue = 0.0f;

	UPROPERTY(ReplicatedUsing = OnRep_KaijuHealthPercent, BlueprintReadOnly, Category = "Goombanics|Kaiju")
	float KaijuHealthPercent = 1.0f;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Goombanics|Kaiju")
	bool bTotalledAchieved = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Goombanics|Kaiju")
	TArray<FGoombanicsWeakPointState> WeakPointStates;

	UPROPERTY(ReplicatedUsing = OnRep_KaijuStaggered, BlueprintReadOnly, Category = "Goombanics|Kaiju")
	bool bKaijuStaggered = false;

	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Goombanics|Scoring")
	FGoombanicsScoreWeights ScoreWeights;

	// Stored for End-of-Round UI.
	UPROPERTY(Replicated, BlueprintReadOnly, Category = "Goombanics|Match")
	EGoombanicsMatchEndReason MatchEndReason = EGoombanicsMatchEndReason::None;

	// Data-only awards snapshot for UI. If replication cost becomes an issue later,
	// we can switch to sending a compact RPC at match end.
	UPROPERTY(ReplicatedUsing = OnRep_EndOfRoundAwards, BlueprintReadOnly, Category = "Goombanics|UI")
	FGoombanicsEndOfRoundAwards EndOfRoundAwards;

	UFUNCTION()
	void OnRep_MatchPhase();

	UFUNCTION()
	void OnRep_DestructionPercent();

	UFUNCTION()
	void OnRep_KaijuHealthPercent();

	UFUNCTION()
	void OnRep_KaijuStaggered();

	UFUNCTION()
	void OnRep_EndOfRoundAwards();
};
