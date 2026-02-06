// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsGameState.h"
#include "Goombanics/Player/GoombanicsPlayerState.h"
#include "Net/UnrealNetwork.h"
#include "GameFramework/PlayerController.h"
#include "Templates/NumericLimits.h"

AGoombanicsGameState::AGoombanicsGameState()
{
	WeakPointStates.Add(FGoombanicsWeakPointState{EGoombanicsWeakPointType::LeftLeg, 100.0f, 100.0f, false});
	WeakPointStates.Add(FGoombanicsWeakPointState{EGoombanicsWeakPointType::RightLeg, 100.0f, 100.0f, false});
	WeakPointStates.Add(FGoombanicsWeakPointState{EGoombanicsWeakPointType::Head, 200.0f, 200.0f, false});
}

void AGoombanicsGameState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoombanicsGameState, MatchPhase);
	DOREPLIFETIME(AGoombanicsGameState, TimeRemaining);
	DOREPLIFETIME(AGoombanicsGameState, DestructionPercent);
	DOREPLIFETIME(AGoombanicsGameState, TotalDestructionValue);
	DOREPLIFETIME(AGoombanicsGameState, CurrentDestructionValue);
	DOREPLIFETIME(AGoombanicsGameState, KaijuHealthPercent);
	DOREPLIFETIME(AGoombanicsGameState, bTotalledAchieved);
	DOREPLIFETIME(AGoombanicsGameState, WeakPointStates);
	DOREPLIFETIME(AGoombanicsGameState, bKaijuStaggered);
	DOREPLIFETIME(AGoombanicsGameState, ScoreWeights);
	DOREPLIFETIME(AGoombanicsGameState, MatchEndReason);
	DOREPLIFETIME(AGoombanicsGameState, EndOfRoundAwards);
}

void AGoombanicsGameState::SetMatchPhase(EGoombanicsMatchPhase NewPhase)
{
	if (MatchPhase != NewPhase)
	{
		MatchPhase = NewPhase;
		OnMatchPhaseChanged.Broadcast(MatchPhase);
	}
}

void AGoombanicsGameState::SetTimeRemaining(float NewTime)
{
	TimeRemaining = FMath::Max(0.0f, NewTime);
}

void AGoombanicsGameState::SetDestructionPercent(float NewPercent)
{
	float OldPercent = DestructionPercent;
	DestructionPercent = FMath::Clamp(NewPercent, 0.0f, 100.0f);
	if (!FMath::IsNearlyEqual(OldPercent, DestructionPercent))
	{
		OnDestructionPercentChanged.Broadcast(DestructionPercent);
	}
}

void AGoombanicsGameState::AddDestructionValue(float Value, APlayerState* Instigator)
{
	CurrentDestructionValue += Value;
	float NewPercent = (TotalDestructionValue > 0.0f) 
		? (CurrentDestructionValue / TotalDestructionValue) * 100.0f 
		: 0.0f;
	SetDestructionPercent(NewPercent);

	if (AGoombanicsPlayerState* PS = Cast<AGoombanicsPlayerState>(Instigator))
	{
		PS->AddCollateralDamage(Value);
	}
}

void AGoombanicsGameState::SetKaijuHealthPercent(float NewPercent)
{
	float OldPercent = KaijuHealthPercent;
	KaijuHealthPercent = FMath::Clamp(NewPercent, 0.0f, 1.0f);
	if (!FMath::IsNearlyEqual(OldPercent, KaijuHealthPercent))
	{
		OnKaijuHealthChanged.Broadcast(KaijuHealthPercent);
	}
}

void AGoombanicsGameState::SetTotalledAchieved(bool bAchieved)
{
	bTotalledAchieved = bAchieved;
}

void AGoombanicsGameState::SetWeakPointStates(const TArray<FGoombanicsWeakPointState>& NewStates)
{
	WeakPointStates = NewStates;
}

void AGoombanicsGameState::UpdateWeakPointState(EGoombanicsWeakPointType Type, float NewHealth, bool bDestroyed, APlayerState* Destroyer)
{
	for (FGoombanicsWeakPointState& State : WeakPointStates)
	{
		if (State.WeakPointType == Type)
		{
			bool bWasDestroyed = State.bIsDestroyed;
			State.CurrentHealth = NewHealth;
			State.bIsDestroyed = bDestroyed;

			if (bDestroyed && !bWasDestroyed)
			{
				OnWeakPointDestroyed.Broadcast(Type, Destroyer);

				if (AGoombanicsPlayerState* PS = Cast<AGoombanicsPlayerState>(Destroyer))
				{
					PS->IncrementWeakPointsDestroyed();
				}
			}
			break;
		}
	}
}

void AGoombanicsGameState::SetKaijuStaggered(bool bStaggered)
{
	if (bKaijuStaggered != bStaggered)
	{
		bKaijuStaggered = bStaggered;
		OnKaijuStaggered.Broadcast(bKaijuStaggered);
	}
}

void AGoombanicsGameState::SetScoreWeights(const FGoombanicsScoreWeights& NewWeights)
{
	ScoreWeights = NewWeights;
}

void AGoombanicsGameState::SetMatchEndReason(EGoombanicsMatchEndReason NewReason)
{
	MatchEndReason = NewReason;
	OnMatchEnded.Broadcast(MatchEndReason);
}

TArray<APlayerState*> AGoombanicsGameState::GetAllPlayerStates() const
{
	TArray<APlayerState*> Result;
	Result.Reserve(PlayerArray.Num());

	for (APlayerState* PS : PlayerArray)
	{
		if (PS)
		{
			Result.Add(PS);
		}
	}

	return Result;
}

bool AGoombanicsGameState::IsLocalPlayerState(APlayerState* PlayerState, APlayerController* LocalPlayerController) const
{
	if (!PlayerState)
	{
		return false;
	}

	UWorld* World = GetWorld();
	if (!World)
	{
		return false;
	}

	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		APlayerController* PC = It->Get();
		if (PC && PC->IsLocalController() && PC->PlayerState == PlayerState)
		{
			return true;
		}
	}

	// Fallback: if caller only has one local PC context.
	return LocalPlayerController && LocalPlayerController->PlayerState == PlayerState;
}

TArray<FGoombanicsScoreboardEntry> AGoombanicsGameState::GetSortedScoreboard(int32 MaxEntries, bool bIncludeAllLocals, APlayerController* LocalPlayerController) const
{
	TArray<AGoombanicsPlayerState*> GoombanicsStates;
	GoombanicsStates.Reserve(PlayerArray.Num());

	for (APlayerState* PS : PlayerArray)
	{
		if (AGoombanicsPlayerState* GPS = Cast<AGoombanicsPlayerState>(PS))
		{
			GoombanicsStates.Add(GPS);
		}
	}

	GoombanicsStates.Sort([](const AGoombanicsPlayerState& A, const AGoombanicsPlayerState& B)
	{
		return A.GetTotalScore() > B.GetTotalScore();
	});

	TArray<FGoombanicsScoreboardEntry> Result;
	TSet<int32> AddedPlayerIds;

	const int32 EffectiveMax = MaxEntries <= 0 ? TNumericLimits<int32>::Max() : MaxEntries;

	for (AGoombanicsPlayerState* PS : GoombanicsStates)
	{
		if (!PS)
		{
			continue;
		}

		const bool bIsLocal = bIncludeAllLocals ? IsLocalPlayerState(PS, LocalPlayerController) : false;
		const bool bCanAdd = (Result.Num() < EffectiveMax) || bIsLocal;

		if (!bCanAdd)
		{
			continue;
		}

		FGoombanicsScoreboardEntry Entry;
		Entry.PlayerName = PS->GetPlayerName();
		Entry.PlayerId = PS->GetPlayerId();
		Entry.TotalScore = PS->GetTotalScore();
		Entry.KaijuDamageDealt = PS->GetKaijuDamageDealt();
		Entry.CollateralDamageScore = PS->GetCollateralDamageScore();
		Entry.WeakPointsDestroyed = PS->GetWeakPointsDestroyed();
		Entry.Deaths = PS->GetDeaths();
		Entry.bIsLocalPlayer = bIsLocal;

		if (!AddedPlayerIds.Contains(Entry.PlayerId))
		{
			AddedPlayerIds.Add(Entry.PlayerId);
			Result.Add(Entry);
		}
	}

	return Result;
}

void AGoombanicsGameState::ComputeEndOfRoundAwards()
{
	TArray<AGoombanicsPlayerState*> GoombanicsStates;
	GoombanicsStates.Reserve(PlayerArray.Num());
	for (APlayerState* PS : PlayerArray)
	{
		if (AGoombanicsPlayerState* GPS = Cast<AGoombanicsPlayerState>(PS))
		{
			GoombanicsStates.Add(GPS);
		}
	}

	// TODO(PlayerControlledKaiju): When a player is the Kaiju, consider role-based filtering for awards:
	// - Humans compete for "Most Kaiju Damage" etc.
	// - Kaiju player competes for collateral-focused awards.

	auto MakeAward = [](EGoombanicsAwardType Type, const AGoombanicsPlayerState* Winner, float Value)
	{
		FGoombanicsAwardResult R;
		R.AwardType = Type;
		R.WinnerPlayerName = Winner ? Winner->GetPlayerName() : FString();
		R.WinnerPlayerId = Winner ? Winner->GetPlayerId() : -1;
		R.Value = Value;
		return R;
	};

	const AGoombanicsPlayerState* MostKaijuDamagePS = nullptr;
	const AGoombanicsPlayerState* MostCollateralDamagePS = nullptr;
	const AGoombanicsPlayerState* MostDeathsPS = nullptr;
	const AGoombanicsPlayerState* MostFinalBlowsPS = nullptr;
	const AGoombanicsPlayerState* WorstDayEverPS = nullptr;

	float MostKaijuDamage = -1.0f;
	float MostCollateralDamage = -1.0f;
	int32 MostDeaths = -1;
	int32 MostFinalBlows = -1;
	float WorstDayEverValue = -1.0f;

	for (const AGoombanicsPlayerState* PS : GoombanicsStates)
	{
		if (!PS)
		{
			continue;
		}

		if (PS->GetKaijuDamageDealt() > MostKaijuDamage)
		{
			MostKaijuDamage = PS->GetKaijuDamageDealt();
			MostKaijuDamagePS = PS;
		}

		if (PS->GetCollateralDamageScore() > MostCollateralDamage)
		{
			MostCollateralDamage = PS->GetCollateralDamageScore();
			MostCollateralDamagePS = PS;
		}

		if (PS->GetDeaths() > MostDeaths)
		{
			MostDeaths = PS->GetDeaths();
			MostDeathsPS = PS;
		}

		if (PS->GetFinalBlowCount() > MostFinalBlows)
		{
			MostFinalBlows = PS->GetFinalBlowCount();
			MostFinalBlowsPS = PS;
		}

		// WorstDayEver placeholder: award to the Kaiju player if present; otherwise highest collateral.
		const bool bIsKaijuRole = PS->GetRole() == EGoombanicsRole::Kaiju;
		const float CandidateCollateral = PS->GetCollateralDamageScore();
		if ((bIsKaijuRole && CandidateCollateral >= 0.0f) || (!WorstDayEverPS && CandidateCollateral >= 0.0f))
		{
			if (CandidateCollateral > WorstDayEverValue)
			{
				WorstDayEverValue = CandidateCollateral;
				WorstDayEverPS = PS;
			}
		}
	}

	FGoombanicsEndOfRoundAwards NewAwards;
	NewAwards.Awards.Add(MakeAward(EGoombanicsAwardType::MostKaijuDamage, MostKaijuDamagePS, FMath::Max(0.0f, MostKaijuDamage)));
	NewAwards.Awards.Add(MakeAward(EGoombanicsAwardType::MostCollateralDamage, MostCollateralDamagePS, FMath::Max(0.0f, MostCollateralDamage)));
	NewAwards.Awards.Add(MakeAward(EGoombanicsAwardType::MostDeaths, MostDeathsPS, (float)FMath::Max(0, MostDeaths)));
	NewAwards.Awards.Add(MakeAward(EGoombanicsAwardType::FinalBlow, MostFinalBlowsPS, (float)FMath::Max(0, MostFinalBlows)));
	NewAwards.Awards.Add(MakeAward(EGoombanicsAwardType::WorstDayEver, WorstDayEverPS, FMath::Max(0.0f, WorstDayEverValue)));

	EndOfRoundAwards = NewAwards;
	OnRep_EndOfRoundAwards();
}

void AGoombanicsGameState::SetTotalDestructionValue(float Value)
{
	TotalDestructionValue = FMath::Max(1.0f, Value);
}

void AGoombanicsGameState::OnRep_MatchPhase()
{
	OnMatchPhaseChanged.Broadcast(MatchPhase);
}

void AGoombanicsGameState::OnRep_DestructionPercent()
{
	OnDestructionPercentChanged.Broadcast(DestructionPercent);
}

void AGoombanicsGameState::OnRep_KaijuHealthPercent()
{
	OnKaijuHealthChanged.Broadcast(KaijuHealthPercent);
}

void AGoombanicsGameState::OnRep_KaijuStaggered()
{
	OnKaijuStaggered.Broadcast(bKaijuStaggered);
}

void AGoombanicsGameState::OnRep_EndOfRoundAwards()
{
	// UI can poll GetEndOfRoundAwards() or bind to MatchEnded delegate.
}
