// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsGameMode.h"
#include "GoombanicsGameState.h"
#include "Goombanics/Player/GoombanicsPlayerState.h"
#include "Goombanics/Player/GoombanicsCharacter.h"
#include "Goombanics/Monster/GoombanicsKaijuPawn.h"
#include "Goombanics/Goombanics.h"
#include "GameFramework/PlayerStart.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

// GameMode responsibilities (server-authoritative):
// - Assign roles at match start / player join.
// - Own match flow (warmup, in-progress, post-round).
//
// TODO(PlayerControlledKaiju): Assign one PlayerState Role=Kaiju and possess the Kaiju pawn.
// TODO(OnlineScaling): Replace simple respawn transform with nav-query near active combat target.

AGoombanicsGameMode::AGoombanicsGameMode()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.bStartWithTickEnabled = true;

	GameStateClass = AGoombanicsGameState::StaticClass();
	PlayerStateClass = AGoombanicsPlayerState::StaticClass();
	DefaultPawnClass = AGoombanicsCharacter::StaticClass();
}

void AGoombanicsGameMode::InitGame(const FString& MapName, const FString& Options, FString& ErrorMessage)
{
	Super::InitGame(MapName, Options, ErrorMessage);
	UE_LOG(LogGoombanics, Log, TEXT("InitGame: Map=%s"), *MapName);
}

void AGoombanicsGameMode::StartPlay()
{
	Super::StartPlay();

	if (AGoombanicsGameState* GS = GetGoombanicsGameState())
	{
		GS->SetMatchPhase(EGoombanicsMatchPhase::Warmup);
		GS->SetTimeRemaining(WarmupDuration);
		GS->SetScoreWeights(ScoreWeights);
	}

	SpawnKaiju();

	UE_LOG(LogGoombanics, Log, TEXT("StartPlay: Warmup phase started"));
}

void AGoombanicsGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	AGoombanicsGameState* GS = GetGoombanicsGameState();
	if (!GS)
	{
		return;
	}

	EGoombanicsMatchPhase CurrentPhase = GS->GetMatchPhase();

	if (CurrentPhase == EGoombanicsMatchPhase::Warmup)
	{
		CurrentWarmupTime += DeltaSeconds;
		GS->SetTimeRemaining(FMath::Max(0.0f, WarmupDuration - CurrentWarmupTime));

		if (CurrentWarmupTime >= WarmupDuration)
		{
			StartMatch();
		}
	}
	else if (CurrentPhase == EGoombanicsMatchPhase::InProgress)
	{
		UpdateMatchTimer(DeltaSeconds);
		CheckMatchEndConditions();
	}

	ProcessPendingRespawns(DeltaSeconds);
}

AActor* AGoombanicsGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	TArray<AActor*> PlayerStarts;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), APlayerStart::StaticClass(), PlayerStarts);

	if (PlayerStarts.Num() > 0)
	{
		int32 Index = FMath::RandRange(0, PlayerStarts.Num() - 1);
		return PlayerStarts[Index];
	}

	return Super::ChoosePlayerStart_Implementation(Player);
}

void AGoombanicsGameMode::HandleStartingNewPlayer_Implementation(APlayerController* NewPlayer)
{
	Super::HandleStartingNewPlayer_Implementation(NewPlayer);

	if (AGoombanicsPlayerState* PS = NewPlayer->GetPlayerState<AGoombanicsPlayerState>())
	{
		PS->SetRole(EGoombanicsRole::Human);
	}
}

void AGoombanicsGameMode::RestartPlayer(AController* NewPlayer)
{
	Super::RestartPlayer(NewPlayer);
}

void AGoombanicsGameMode::StartMatch()
{
	if (bMatchStarted)
	{
		return;
	}

	bMatchStarted = true;

	if (AGoombanicsGameState* GS = GetGoombanicsGameState())
	{
		GS->SetMatchPhase(EGoombanicsMatchPhase::InProgress);
		GS->SetTimeRemaining(MatchDuration);
	}

	UE_LOG(LogGoombanics, Log, TEXT("Match started!"));
}

void AGoombanicsGameMode::EndMatch(EGoombanicsMatchEndReason Reason)
{
	if (AGoombanicsGameState* GS = GetGoombanicsGameState())
	{
		if (GS->GetMatchPhase() == EGoombanicsMatchPhase::PostRound)
		{
			return;
		}

		GS->SetMatchPhase(EGoombanicsMatchPhase::PostRound);
		GS->OnMatchEnded.Broadcast(Reason);

		for (FConstPlayerControllerIterator It = GetWorld()->GetPlayerControllerIterator(); It; ++It)
		{
			if (APlayerController* PC = It->Get())
			{
				if (AGoombanicsPlayerState* PS = PC->GetPlayerState<AGoombanicsPlayerState>())
				{
					PS->CalculateFinalScore();
				}
			}
		}
	}

	UE_LOG(LogGoombanics, Log, TEXT("Match ended: %d"), static_cast<int32>(Reason));
}

void AGoombanicsGameMode::CreateLocalPlayers(int32 NumPlayers)
{
	NumPlayers = FMath::Clamp(NumPlayers, 1, 4);

	UGameInstance* GI = GetGameInstance();
	if (!GI)
	{
		UE_LOG(LogGoombanics, Error, TEXT("CreateLocalPlayers: No GameInstance"));
		return;
	}

	int32 CurrentLocalPlayers = GI->GetNumLocalPlayers();

	for (int32 i = CurrentLocalPlayers; i < NumPlayers; ++i)
	{
		FString Error;
		ULocalPlayer* NewPlayer = GI->CreateLocalPlayer(i, Error, true);
		if (NewPlayer)
		{
			UE_LOG(LogGoombanics, Log, TEXT("Created local player %d"), i);
		}
		else
		{
			UE_LOG(LogGoombanics, Error, TEXT("Failed to create local player %d: %s"), i, *Error);
		}
	}
}

void AGoombanicsGameMode::RequestPlayerRespawn(AController* Controller)
{
	if (!Controller)
	{
		return;
	}

	if (AGoombanicsGameState* GS = GetGoombanicsGameState())
	{
		if (GS->GetMatchPhase() != EGoombanicsMatchPhase::InProgress)
		{
			return;
		}
	}

	if (!PendingRespawns.Contains(Controller))
	{
		PendingRespawns.Add(Controller, RespawnDelay);

		if (AGoombanicsPlayerState* PS = Controller->GetPlayerState<AGoombanicsPlayerState>())
		{
			PS->IncrementDeaths();
		}

		UE_LOG(LogGoombanics, Log, TEXT("Player respawn requested, delay: %.1f"), RespawnDelay);
	}
}

void AGoombanicsGameMode::SpawnKaiju()
{
	if (!KaijuPawnClass)
	{
		UE_LOG(LogGoombanics, Warning, TEXT("SpawnKaiju: No KaijuPawnClass set"));
		return;
	}

	FVector SpawnLocation = FVector(0.0f, 0.0f, 500.0f);
	FRotator SpawnRotation = FRotator::ZeroRotator;

	TArray<AActor*> KaijuSpawns;
	UGameplayStatics::GetAllActorsWithTag(GetWorld(), FName("KaijuSpawn"), KaijuSpawns);
	if (KaijuSpawns.Num() > 0)
	{
		SpawnLocation = KaijuSpawns[0]->GetActorLocation();
		SpawnRotation = KaijuSpawns[0]->GetActorRotation();
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

	ActiveKaiju = GetWorld()->SpawnActor<AGoombanicsKaijuPawn>(KaijuPawnClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (ActiveKaiju)
	{
		UE_LOG(LogGoombanics, Log, TEXT("Kaiju spawned at %s"), *SpawnLocation.ToString());
	}
}

void AGoombanicsGameMode::SetPlayerRole(APlayerState* PlayerState, EGoombanicsRole NewRole)
{
	if (AGoombanicsPlayerState* PS = Cast<AGoombanicsPlayerState>(PlayerState))
	{
		PS->SetRole(NewRole);
		return;
	}
}

EGoombanicsRole AGoombanicsGameMode::GetPlayerRole(const APlayerState* PlayerState) const
{
	if (const AGoombanicsPlayerState* PS = Cast<AGoombanicsPlayerState>(PlayerState))
	{
		return PS->GetRole();
	}

	return EGoombanicsRole::Human;
}

AGoombanicsGameState* AGoombanicsGameMode::GetGoombanicsGameState() const
{
	return GetGameState<AGoombanicsGameState>();
}

void AGoombanicsGameMode::CheckMatchEndConditions()
{
	AGoombanicsGameState* GS = GetGoombanicsGameState();
	if (!GS || GS->GetMatchPhase() != EGoombanicsMatchPhase::InProgress)
	{
		return;
	}

	if (GS->GetKaijuHealthPercent() <= 0.0f)
	{
		EndMatch(EGoombanicsMatchEndReason::KaijuDefeated);
		return;
	}

	if (GS->GetDestructionPercent() >= DestructionThreshold)
	{
		EndMatch(EGoombanicsMatchEndReason::CityDestroyed);
		return;
	}

	if (GS->GetTimeRemaining() <= 0.0f)
	{
		EndMatch(EGoombanicsMatchEndReason::TimerExpired);
		return;
	}
}

void AGoombanicsGameMode::UpdateMatchTimer(float DeltaSeconds)
{
	if (AGoombanicsGameState* GS = GetGoombanicsGameState())
	{
		float NewTime = FMath::Max(0.0f, GS->GetTimeRemaining() - DeltaSeconds);
		GS->SetTimeRemaining(NewTime);
	}
}

void AGoombanicsGameMode::ProcessPendingRespawns(float DeltaSeconds)
{
	TArray<AController*> ToRespawn;

	for (auto& Pair : PendingRespawns)
	{
		Pair.Value -= DeltaSeconds;
		if (Pair.Value <= 0.0f)
		{
			ToRespawn.Add(Pair.Key);
		}
	}

	for (AController* Controller : ToRespawn)
	{
		PendingRespawns.Remove(Controller);
		RestartPlayer(Controller);
	}
}

FTransform AGoombanicsGameMode::GetRespawnTransform(AController* Controller) const
{
	AActor* PlayerStart = ChoosePlayerStart(Controller);
	if (PlayerStart)
	{
		return PlayerStart->GetActorTransform();
	}
	return FTransform::Identity;
}
