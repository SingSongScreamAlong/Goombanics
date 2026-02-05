// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsHUD.h"
#include "GoombanicsHUDWidget.h"
#include "Goombanics/Goombanics.h"
#include "Blueprint/UserWidget.h"

AGoombanicsHUD::AGoombanicsHUD()
{
}

void AGoombanicsHUD::BeginPlay()
{
	Super::BeginPlay();

	if (HUDWidgetClass)
	{
		HUDWidget = CreateWidget<UGoombanicsHUDWidget>(GetOwningPlayerController(), HUDWidgetClass);
		if (HUDWidget)
		{
			HUDWidget->AddToPlayerScreen();
			UE_LOG(LogGoombanics, Log, TEXT("HUD widget created"));
		}
	}
}

void AGoombanicsHUD::ShowHUD()
{
	if (HUDWidget)
	{
		HUDWidget->SetVisibility(ESlateVisibility::Visible);
	}
}

void AGoombanicsHUD::HideHUD()
{
	if (HUDWidget)
	{
		HUDWidget->SetVisibility(ESlateVisibility::Hidden);
	}
}

void AGoombanicsHUD::ShowEndOfRoundScreen(EGoombanicsMatchEndReason EndReason)
{
	if (HUDWidget)
	{
		HUDWidget->ShowEndOfRoundScreen(EndReason);
	}
}
