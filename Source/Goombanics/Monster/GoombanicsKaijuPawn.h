// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GoombanicsMonsterBase.h"
#include "GoombanicsKaijuPawn.generated.h"

class UBoxComponent;

UENUM(BlueprintType)
enum class EKaijuAIState : uint8
{
	Idle		UMETA(DisplayName = "Idle"),
	Pursuing	UMETA(DisplayName = "Pursuing"),
	Attacking	UMETA(DisplayName = "Attacking"),
	Staggered	UMETA(DisplayName = "Staggered"),
	Dead		UMETA(DisplayName = "Dead")
};

UCLASS()
class GOOMBANICS_API AGoombanicsKaijuPawn : public AGoombanicsMonsterBase
{
	GENERATED_BODY()

public:
	AGoombanicsKaijuPawn();

	// -----------------------------------------------------------------------------
	// Feel / Readability (Design Intent - WRITE-ONLY)
	//
	// Player perception goals:
	// - Massive: scale and momentum.
	// - Unfair at first: early mistakes are punished.
	// - Learnable: every attack has a clear telegraph and consistent timing.
	//
	// Telegraphing:
	// - Each attack should have a wind-up pose + audio cue.
	// - Impact zones should be predictable.
	//
	// Stagger readability:
	// - Must be unmissable (pose collapse + cue + HUD indicator).
	// - Head weak point becomes the reward window.
	//
	// TODO(Phase2-Windows): Externalize Kaiju tuning (health/stagger duration/head multiplier)
	// into a UDataAsset (see GoombanicsGameplayTuningData).
	// TODO(Chaos): On heavy attacks, trigger controlled Chaos setpieces without breaking readability.
	// -----------------------------------------------------------------------------

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;
	virtual void TriggerStagger_Implementation() override;
	virtual void EndStagger_Implementation() override;

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Kaiju|AI")
	void SetAIEnabled(bool bEnabled);

	UFUNCTION(BlueprintPure, Category = "Goombanics|Kaiju|AI")
	EKaijuAIState GetCurrentAIState() const { return CurrentAIState; }

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Kaiju|Attacks")
	void PerformStompAttack();

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Kaiju|Attacks")
	void PerformSweepAttack();

	UFUNCTION(BlueprintPure, Category = "Goombanics|Kaiju")
	AActor* GetCurrentTarget() const { return CurrentTarget.Get(); }

protected:
	virtual void UpdateAI(float DeltaTime);
	virtual void UpdatePursuit(float DeltaTime);
	virtual void UpdateAttack(float DeltaTime);
	virtual AActor* FindNearestPlayer() const;
	virtual void SelectAttack();
	virtual void ApplyAttackDamage(const FVector& Center, float Radius, float Damage);
	virtual void DamageNearbyDestructibles(const FVector& Center, float Radius);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goombanics|Components")
	TObjectPtr<UBoxComponent> LeftLegHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goombanics|Components")
	TObjectPtr<UBoxComponent> RightLegHitbox;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goombanics|Components")
	TObjectPtr<UBoxComponent> HeadHitbox;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Kaiju|AI")
	float PursuitSpeed = 300.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Kaiju|AI")
	float AttackRange = 500.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Kaiju|AI")
	float AttackCooldown = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Kaiju|AI")
	float TargetUpdateInterval = 0.5f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Kaiju|Attacks")
	float StompDamage = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Kaiju|Attacks")
	float StompRadius = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Kaiju|Attacks")
	float SweepDamage = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Kaiju|Attacks")
	float SweepRadius = 600.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Kaiju|Attacks")
	float DestructionRadius = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Kaiju|Attacks")
	float AttackDuration = 1.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Goombanics|Kaiju|AI")
	EKaijuAIState CurrentAIState = EKaijuAIState::Idle;

	UPROPERTY(BlueprintReadOnly, Category = "Goombanics|Kaiju|AI")
	TWeakObjectPtr<AActor> CurrentTarget;

	bool bAIEnabled = true;
	float AttackCooldownRemaining = 0.0f;
	float TargetUpdateTimer = 0.0f;
	float AttackTimeRemaining = 0.0f;
	bool bIsAttacking = false;
};
