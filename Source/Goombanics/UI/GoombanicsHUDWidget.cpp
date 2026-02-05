// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsHUDWidget.h"
#include "Goombanics/Core/GoombanicsGameState.h"
#include "Goombanics/Player/GoombanicsPlayerState.h"
#include "Goombanics/Goombanics.h"
#include "Components/TextBlock.h"
#include "Components/ProgressBar.h"
#include "Components/VerticalBox.h"
#include "Components/Overlay.h"
#include "Kismet/GameplayStatics.h"

void UGoombanicsHUDWidget::NativeConstruct()
{
	Super::NativeConstruct();

	if (EndOfRoundOverlay)
	{
		EndOfRoundOverlay->SetVisibility(ESlateVisibility::Collapsed);
	}

	if (StaggerText)
	{
		StaggerText->SetVisibility(ESlateVisibility::Collapsed);
	}

	BindToGameState();
}

void UGoombanicsHUDWidget::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	if (!bBoundToGameState)
	{
		BindToGameState();
	}

	UpdateFromGameState();
}

void UGoombanicsHUDWidget::UpdateTimer(float TimeRemaining)
{
	if (TimerText)
	{
		int32 Minutes = FMath::FloorToInt(TimeRemaining / 60.0f);
		int32 Seconds = FMath::FloorToInt(FMath::Fmod(TimeRemaining, 60.0f));
		TimerText->SetText(FText::FromString(FString::Printf(TEXT("%02d:%02d"), Minutes, Seconds)));
	}
}

void UGoombanicsHUDWidget::UpdateDestructionMeter(float Percent)
{
	if (DestructionBar)
	{
		DestructionBar->SetPercent(Percent / 100.0f);
	}

	if (DestructionText)
	{
		DestructionText->SetText(FText::FromString(FString::Printf(TEXT("DESTRUCTION: %.0f%%"), Percent)));
	}
}

void UGoombanicsHUDWidget::UpdateKaijuHealth(float HealthPercent)
{
	if (KaijuHealthBar)
	{
		KaijuHealthBar->SetPercent(HealthPercent);
	}

	if (KaijuHealthText)
	{
		KaijuHealthText->SetText(FText::FromString(FString::Printf(TEXT("KAIJU: %.0f%%"), HealthPercent * 100.0f)));
	}
}

void UGoombanicsHUDWidget::UpdateWeakPoints(const TArray<FGoombanicsWeakPointState>& WeakPoints)
{
	for (const FGoombanicsWeakPointState& WP : WeakPoints)
	{
		UProgressBar* TargetBar = nullptr;

		switch (WP.WeakPointType)
		{
		case EGoombanicsWeakPointType::LeftLeg:
			TargetBar = LeftLegBar;
			break;
		case EGoombanicsWeakPointType::RightLeg:
			TargetBar = RightLegBar;
			break;
		case EGoombanicsWeakPointType::Head:
			TargetBar = HeadBar;
			break;
		default:
			break;
		}

		if (TargetBar)
		{
			TargetBar->SetPercent(WP.GetHealthPercent());

			if (WP.bIsDestroyed)
			{
				TargetBar->SetFillColorAndOpacity(FLinearColor::Red);
			}
			else
			{
				TargetBar->SetFillColorAndOpacity(FLinearColor::Green);
			}
		}
	}
}

void UGoombanicsHUDWidget::UpdateAmmo(int32 CurrentAmmo, int32 MaxAmmo)
{
	if (AmmoText)
	{
		AmmoText->SetText(FText::FromString(FString::Printf(TEXT("%d / %d"), CurrentAmmo, MaxAmmo)));
	}
}

void UGoombanicsHUDWidget::UpdatePlayerHealth(float HealthPercent)
{
	if (PlayerHealthBar)
	{
		PlayerHealthBar->SetPercent(HealthPercent);
	}
}

void UGoombanicsHUDWidget::ShowStaggerIndicator(bool bShow)
{
	if (StaggerText)
	{
		StaggerText->SetVisibility(bShow ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
	}
}

void UGoombanicsHUDWidget::ShowEndOfRoundScreen(EGoombanicsMatchEndReason EndReason)
{
	if (EndOfRoundOverlay)
	{
		EndOfRoundOverlay->SetVisibility(ESlateVisibility::Visible);
	}

	if (EndReasonText)
	{
		FString ReasonString;
		switch (EndReason)
		{
		case EGoombanicsMatchEndReason::KaijuDefeated:
			ReasonString = TEXT("KAIJU DEFEATED!");
			break;
		case EGoombanicsMatchEndReason::CityDestroyed:
			ReasonString = TEXT("CITY DESTROYED!");
			break;
		case EGoombanicsMatchEndReason::TimerExpired:
			ReasonString = TEXT("TIME'S UP!");
			break;
		default:
			ReasonString = TEXT("MATCH ENDED");
			break;
		}
		EndReasonText->SetText(FText::FromString(ReasonString));
	}

	RefreshScoreboard();
}

void UGoombanicsHUDWidget::RefreshScoreboard()
{
	if (!FinalScoresBox)
	{
		return;
	}

	FinalScoresBox->ClearChildren();

	UWorld* World = GetWorld();
	if (!World)
	{
		return;
	}

	TArray<AGoombanicsPlayerState*> PlayerStates;
	for (FConstPlayerControllerIterator It = World->GetPlayerControllerIterator(); It; ++It)
	{
		if (APlayerController* PC = It->Get())
		{
			if (AGoombanicsPlayerState* PS = PC->GetPlayerState<AGoombanicsPlayerState>())
			{
				PlayerStates.Add(PS);
			}
		}
	}

	PlayerStates.Sort([](const AGoombanicsPlayerState& A, const AGoombanicsPlayerState& B)
	{
		return A.GetTotalScore() > B.GetTotalScore();
	});

	UE_LOG(LogGoombanics, Log, TEXT("Scoreboard refreshed with %d players"), PlayerStates.Num());
}

void UGoombanicsHUDWidget::BindToGameState()
{
	AGoombanicsGameState* GS = Cast<AGoombanicsGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!GS)
	{
		return;
	}

	GS->OnMatchPhaseChanged.AddDynamic(this, &UGoombanicsHUDWidget::OnMatchPhaseChanged);
	GS->OnDestructionPercentChanged.AddDynamic(this, &UGoombanicsHUDWidget::OnDestructionPercentChanged);
	GS->OnKaijuHealthChanged.AddDynamic(this, &UGoombanicsHUDWidget::OnKaijuHealthChanged);
	GS->OnKaijuStaggered.AddDynamic(this, &UGoombanicsHUDWidget::OnKaijuStaggered);
	GS->OnMatchEnded.AddDynamic(this, &UGoombanicsHUDWidget::OnMatchEnded);

	bBoundToGameState = true;
	UE_LOG(LogGoombanics, Log, TEXT("HUD bound to GameState"));
}

void UGoombanicsHUDWidget::UpdateFromGameState()
{
	AGoombanicsGameState* GS = Cast<AGoombanicsGameState>(UGameplayStatics::GetGameState(GetWorld()));
	if (!GS)
	{
		return;
	}

	UpdateTimer(GS->GetTimeRemaining());
	UpdateDestructionMeter(GS->GetDestructionPercent());
	UpdateKaijuHealth(GS->GetKaijuHealthPercent());
	UpdateWeakPoints(GS->GetWeakPointStates());
	ShowStaggerIndicator(GS->IsKaijuStaggered());
}

void UGoombanicsHUDWidget::OnMatchPhaseChanged(EGoombanicsMatchPhase NewPhase)
{
	UE_LOG(LogGoombanics, Log, TEXT("HUD: Match phase changed to %d"), static_cast<int32>(NewPhase));
}

void UGoombanicsHUDWidget::OnDestructionPercentChanged(float NewPercent)
{
	UpdateDestructionMeter(NewPercent);
}

void UGoombanicsHUDWidget::OnKaijuHealthChanged(float NewHealthPercent)
{
	UpdateKaijuHealth(NewHealthPercent);
}

void UGoombanicsHUDWidget::OnKaijuStaggered(bool bIsStaggered)
{
	ShowStaggerIndicator(bIsStaggered);
}

void UGoombanicsHUDWidget::OnMatchEnded(EGoombanicsMatchEndReason EndReason)
{
	ShowEndOfRoundScreen(EndReason);
}
