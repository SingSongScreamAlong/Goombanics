// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoombanicsBreakableActor.generated.h"

class UStaticMeshComponent;
class UBoxComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnBreakableDestroyed, AGoombanicsBreakableActor*, Breakable, APlayerState*, Destroyer);

UCLASS()
class GOOMBANICS_API AGoombanicsBreakableActor : public AActor
{
	GENERATED_BODY()

public:
	AGoombanicsBreakableActor();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Destruction")
	void Break(APlayerState* Instigator);

	UFUNCTION(BlueprintPure, Category = "Goombanics|Destruction")
	bool IsBroken() const { return bIsBroken; }

	UFUNCTION(BlueprintPure, Category = "Goombanics|Destruction")
	float GetDestructionValue() const { return DestructionValue; }

	UPROPERTY(BlueprintAssignable, Category = "Goombanics|Destruction|Events")
	FOnBreakableDestroyed OnBreakableDestroyed;

protected:
	virtual void OnBroken(APlayerState* Instigator);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goombanics|Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goombanics|Components")
	TObjectPtr<UBoxComponent> CollisionComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goombanics|Destruction")
	float DestructionValue = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goombanics|Destruction")
	bool bContributesToDestructionMeter = true;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Goombanics|Destruction")
	TObjectPtr<UStaticMesh> BrokenMesh;

	UPROPERTY(BlueprintReadOnly, Category = "Goombanics|Destruction")
	bool bIsBroken = false;
};
