// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoombanicsMonsterInterface.h"
#include "GoombanicsMonsterBase.generated.h"

class USkeletalMeshComponent;
class UCapsuleComponent;
class UFloatingPawnMovement;

UCLASS(Abstract)
class GOOMBANICS_API AGoombanicsMonsterBase : public APawn, public IGoombanicsMonsterInterface
{
	GENERATED_BODY()

public:
	AGoombanicsMonsterBase();

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void PossessedBy(AController* NewController) override;
	virtual void UnPossessed() override;

	// Possession path notes:
	// - Weak point + health logic remains on the Pawn and must behave the same for AI and player controllers.
	// - PossessedBy/UnPossessed should be the single choke point for switching ability gating.
	//
	// TODO(PlayerControlledKaiju):
	// - Add a Kaiju input component and bind actions only when possessed by a PlayerController.
	// - Keep AI logic available when possessed by an AIController.
	// - Prevent duplicate behavior (AI + player) by gating on bIsControlledByPlayer.
	//
	// TODO(PlayerControlledKaiju): Allow mid-match possession swap without breaking weak point state.

	virtual float GetMonsterHealthPercent_Implementation() const override;
	virtual void ApplyDamageToMonster_Implementation(float Damage, AController* Instigator, AActor* DamageCauser) override;
	virtual void ApplyDamageToWeakPoint_Implementation(EGoombanicsWeakPointType WeakPointType, float Damage, AController* Instigator) override;
	virtual bool IsMonsterStaggered_Implementation() const override;
	virtual void TriggerStagger_Implementation() override;
	virtual void EndStagger_Implementation() override;
	virtual TArray<FGoombanicsWeakPointState> GetWeakPointStates_Implementation() const override;
	virtual bool IsWeakPointExposed_Implementation(EGoombanicsWeakPointType WeakPointType) const override;
	virtual void OnPossessedByPlayer_Implementation(AController* NewController) override;
	virtual void OnPossessedByAI_Implementation(AController* NewController) override;

	UFUNCTION(BlueprintPure, Category = "Goombanics|Monster")
	bool IsControlledByPlayer() const { return bIsControlledByPlayer; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Monster")
	void Die(AController* Killer);

protected:
	virtual void UpdateStagger(float DeltaTime);
	virtual void CheckStaggerConditions();
	virtual void OnWeakPointDestroyed(EGoombanicsWeakPointType WeakPointType, AController* Destroyer);
	virtual void OnDeath(AController* Killer);
	virtual void UpdateGameState();

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goombanics|Components")
	TObjectPtr<UCapsuleComponent> CapsuleComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goombanics|Components")
	TObjectPtr<USkeletalMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goombanics|Components")
	TObjectPtr<UFloatingPawnMovement> MovementComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Health")
	float MaxHealth = 5000.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Goombanics|Health")
	float CurrentHealth = 5000.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|WeakPoints")
	TArray<FGoombanicsWeakPointState> WeakPoints;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Stagger")
	float StaggerDuration = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Stagger")
	float HeadDamageMultiplierDuringStagger = 2.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Goombanics|Stagger")
	bool bIsStaggered = false;

	UPROPERTY(BlueprintReadOnly, Category = "Goombanics|Stagger")
	float StaggerTimeRemaining = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Goombanics|State")
	bool bIsControlledByPlayer = false;

	UPROPERTY(BlueprintReadOnly, Category = "Goombanics|State")
	bool bIsDead = false;
};
