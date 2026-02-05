// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/HUD.h"
#include "Goombanics/Core/GoombanicsTypes.h"
#include "GoombanicsHUD.generated.h"

class UGoombanicsHUDWidget;

UCLASS()
class GOOMBANICS_API AGoombanicsHUD : public AHUD
{
	GENERATED_BODY()

public:
	AGoombanicsHUD();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Goombanics|HUD")
	void ShowHUD();

	UFUNCTION(BlueprintCallable, Category = "Goombanics|HUD")
	void HideHUD();

	UFUNCTION(BlueprintCallable, Category = "Goombanics|HUD")
	void ShowEndOfRoundScreen(EGoombanicsMatchEndReason EndReason);

	UFUNCTION(BlueprintPure, Category = "Goombanics|HUD")
	UGoombanicsHUDWidget* GetHUDWidget() const { return HUDWidget; }

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|HUD")
	TSubclassOf<UGoombanicsHUDWidget> HUDWidgetClass;

	UPROPERTY(BlueprintReadOnly, Category = "Goombanics|HUD")
	TObjectPtr<UGoombanicsHUDWidget> HUDWidget;
};
