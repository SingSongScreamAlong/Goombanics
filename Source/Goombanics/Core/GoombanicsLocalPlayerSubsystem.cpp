// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsLocalPlayerSubsystem.h"
#include "Goombanics/Goombanics.h"
#include "EnhancedInputSubsystems.h"
#include "InputMappingContext.h"
#include "Engine/LocalPlayer.h"
#include "GameFramework/PlayerController.h"

void UGoombanicsLocalPlayerSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
	UE_LOG(LogGoombanics, Log, TEXT("LocalPlayerSubsystem initialized for player index %d"), GetLocalPlayerIndex());
}

void UGoombanicsLocalPlayerSubsystem::Deinitialize()
{
	Super::Deinitialize();
}

void UGoombanicsLocalPlayerSubsystem::SetupInputForPlayer(APlayerController* PlayerController)
{
	if (!PlayerController)
	{
		return;
	}

	ULocalPlayer* LP = GetLocalPlayer();
	if (!LP)
	{
		return;
	}

	if (UEnhancedInputLocalPlayerSubsystem* InputSubsystem = LP->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>())
	{
		if (DefaultMappingContext)
		{
			InputSubsystem->AddMappingContext(DefaultMappingContext, 0);
		}
	}

	UE_LOG(LogGoombanics, Log, TEXT("Input setup for local player %d"), GetLocalPlayerIndex());
}

int32 UGoombanicsLocalPlayerSubsystem::GetLocalPlayerIndex() const
{
	if (ULocalPlayer* LP = GetLocalPlayer())
	{
		if (UGameInstance* GI = LP->GetGameInstance())
		{
			return GI->GetLocalPlayers().Find(LP);
		}
	}
	return INDEX_NONE;
}

bool UGoombanicsLocalPlayerSubsystem::IsFirstLocalPlayer() const
{
	return GetLocalPlayerIndex() == 0;
}
