// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GoombanicsTypes.h"
#include "GoombanicsGameMode.generated.h"

class AGoombanicsGameState;
class AGoombanicsPlayerState;
class AGoombanicsKaijuPawn;

UCLASS()
class GOOMBANICS_API AGoombanicsGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AGoombanicsGameMode();

	virtual void InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage) override;
	virtual void StartPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override;
	virtual void HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer) override;
	virtual void RestartPlayer(AController* NewPlayer) override;

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Match")
	void StartMatch();

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Match")
	void EndMatch(EGoombanicsMatchEndReason Reason);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|LocalPlayers")
	void CreateLocalPlayers(int32 NumPlayers);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Respawn")
	void RequestPlayerRespawn(AController* Controller);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Kaiju")
	void SpawnKaiju();

	// -----------------------------------------------------------------------------
	// Role assignment API (WRITE-ONLY scaffolding)
	//
	// TODO(PlayerControlledKaiju): Role assignment can happen:
	// - Pre-match (e.g. one player starts as Kaiju)
	// - Mid-match possession swap (optional future)
	// -----------------------------------------------------------------------------
	UFUNCTION(BlueprintCallable, Category = "Goombanics|Role")
	void SetPlayerRole(APlayerState* PlayerState, EGoombanicsRole NewRole);

	UFUNCTION(BlueprintPure, Category = "Goombanics|Role")
	EGoombanicsRole GetPlayerRole(const APlayerState* PlayerState) const;

	UFUNCTION(BlueprintPure, Category = "Goombanics|Match")
	AGoombanicsGameState* GetGoombanicsGameState() const;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Config")
	float MatchDuration = 180.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Config")
	float WarmupDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Config")
	float RespawnDelay = 4.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Config")
	float DestructionThreshold = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Config")
	TSubclassOf<AGoombanicsKaijuPawn> KaijuPawnClass;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Config")
	FGoombanicsScoreWeights ScoreWeights;

protected:
	virtual void CheckMatchEndConditions();
	virtual void UpdateMatchTimer(float DeltaSeconds);
	virtual void ProcessPendingRespawns(float DeltaSeconds);
	virtual FTransform GetRespawnTransform(AController* Controller) const;

	UPROPERTY()
	TObjectPtr<AGoombanicsKaijuPawn> ActiveKaiju;

	UPROPERTY()
	TMap<AController*, float> PendingRespawns;

	float CurrentWarmupTime = 0.0f;
	bool bMatchStarted = false;
};
