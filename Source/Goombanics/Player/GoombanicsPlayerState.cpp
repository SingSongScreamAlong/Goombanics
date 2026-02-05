// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsPlayerState.h"
#include "Goombanics/Core/GoombanicsGameState.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"

AGoombanicsPlayerState::AGoombanicsPlayerState()
{
}

void AGoombanicsPlayerState::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoombanicsPlayerState, Role);
	DOREPLIFETIME(AGoombanicsPlayerState, ScoreData);
}

void AGoombanicsPlayerState::SetRole(EGoombanicsRole NewRole)
{
	Role = NewRole;
}

void AGoombanicsPlayerState::AddKaijuDamage(float Damage)
{
	ScoreData.KaijuDamageDealt += Damage;
}

void AGoombanicsPlayerState::AddCollateralDamage(float Damage)
{
	ScoreData.CollateralDamageScore += Damage;
}

void AGoombanicsPlayerState::IncrementWeakPointsDestroyed()
{
	ScoreData.WeakPointsDestroyed++;
}

void AGoombanicsPlayerState::IncrementDeaths()
{
	ScoreData.Deaths++;
}

void AGoombanicsPlayerState::IncrementFinalBlowCount()
{
	ScoreData.FinalBlowCount++;
}

void AGoombanicsPlayerState::CalculateFinalScore()
{
	if (UWorld* World = GetWorld())
	{
		if (AGoombanicsGameState* GS = Cast<AGoombanicsGameState>(UGameplayStatics::GetGameState(World)))
		{
			ScoreData.CalculateTotalScore(GS->GetScoreWeights());
		}
	}
}

void AGoombanicsPlayerState::ResetScore()
{
	ScoreData = FGoombanicsPlayerScoreData();
}
