// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/LocalPlayerSubsystem.h"
#include "GoombanicsLocalPlayerSubsystem.generated.h"

class UInputMappingContext;

UCLASS()
class GOOMBANICS_API UGoombanicsLocalPlayerSubsystem : public ULocalPlayerSubsystem
{
	GENERATED_BODY()

public:
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Input")
	void SetupInputForPlayer(APlayerController* PlayerController);

	UFUNCTION(BlueprintPure, Category = "Goombanics|LocalPlayer")
	int32 GetLocalPlayerIndex() const;

	UFUNCTION(BlueprintPure, Category = "Goombanics|LocalPlayer")
	bool IsFirstLocalPlayer() const;

protected:
	UPROPERTY()
	TObjectPtr<UInputMappingContext> DefaultMappingContext;
};
