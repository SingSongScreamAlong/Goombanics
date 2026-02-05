// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Goombanics/Core/GoombanicsTypes.h"
#include "GoombanicsHUDWidget.generated.h"

class UTextBlock;
class UProgressBar;
class UVerticalBox;
class UOverlay;

UCLASS()
class GOOMBANICS_API UGoombanicsHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	virtual void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	UFUNCTION(BlueprintCallable, Category = "Goombanics|HUD")
	void UpdateTimer(float TimeRemaining);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|HUD")
	void UpdateDestructionMeter(float Percent);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|HUD")
	void UpdateKaijuHealth(float HealthPercent);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|HUD")
	void UpdateWeakPoints(const TArray<FGoombanicsWeakPointState>& WeakPoints);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|HUD")
	void UpdateAmmo(int32 CurrentAmmo, int32 MaxAmmo);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|HUD")
	void UpdatePlayerHealth(float HealthPercent);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|HUD")
	void ShowStaggerIndicator(bool bShow);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|HUD")
	void ShowEndOfRoundScreen(EGoombanicsMatchEndReason EndReason);

	UFUNCTION(BlueprintCallable, Category = "Goombanics|HUD")
	void RefreshScoreboard();

protected:
	void BindToGameState();
	void UpdateFromGameState();

	UFUNCTION()
	void OnMatchPhaseChanged(EGoombanicsMatchPhase NewPhase);

	UFUNCTION()
	void OnDestructionPercentChanged(float NewPercent);

	UFUNCTION()
	void OnKaijuHealthChanged(float NewHealthPercent);

	UFUNCTION()
	void OnKaijuStaggered(bool bIsStaggered);

	UFUNCTION()
	void OnMatchEnded(EGoombanicsMatchEndReason EndReason);

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UTextBlock> TimerText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UProgressBar> DestructionBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UTextBlock> DestructionText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UProgressBar> KaijuHealthBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UTextBlock> KaijuHealthText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UProgressBar> LeftLegBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UProgressBar> RightLegBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UProgressBar> HeadBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UTextBlock> AmmoText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UProgressBar> PlayerHealthBar;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UTextBlock> StaggerText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UVerticalBox> ScoreboardBox;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UOverlay> EndOfRoundOverlay;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UTextBlock> EndReasonText;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UVerticalBox> FinalScoresBox;

	UPROPERTY(meta = (BindWidget), BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UVerticalBox> AwardsBox;

	bool bBoundToGameState = false;
};
