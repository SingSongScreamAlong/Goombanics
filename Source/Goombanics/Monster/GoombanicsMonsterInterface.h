// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "Goombanics/Core/GoombanicsTypes.h"
#include "GoombanicsMonsterInterface.generated.h"

UINTERFACE(MinimalAPI, Blueprintable)
class UGoombanicsMonsterInterface : public UInterface
{
	GENERATED_BODY()
};

class GOOMBANICS_API IGoombanicsMonsterInterface
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Goombanics|Monster")
	float GetMonsterHealthPercent() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Goombanics|Monster")
	void ApplyDamageToMonster(float Damage, AController* Instigator, AActor* DamageCauser);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Goombanics|Monster")
	void ApplyDamageToWeakPoint(EGoombanicsWeakPointType WeakPointType, float Damage, AController* Instigator);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Goombanics|Monster")
	bool IsMonsterStaggered() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Goombanics|Monster")
	void TriggerStagger();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Goombanics|Monster")
	void EndStagger();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Goombanics|Monster")
	TArray<FGoombanicsWeakPointState> GetWeakPointStates() const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Goombanics|Monster")
	bool IsWeakPointExposed(EGoombanicsWeakPointType WeakPointType) const;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Goombanics|Monster")
	void OnPossessedByPlayer(AController* NewController);

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "Goombanics|Monster")
	void OnPossessedByAI(AController* NewController);
};
