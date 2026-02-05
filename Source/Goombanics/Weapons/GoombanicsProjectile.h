// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GoombanicsProjectile.generated.h"

class USphereComponent;
class UProjectileMovementComponent;
class UStaticMeshComponent;

UCLASS()
class GOOMBANICS_API AGoombanicsProjectile : public AActor
{
	GENERATED_BODY()

public:
	AGoombanicsProjectile();

	virtual void BeginPlay() override;

	UFUNCTION(BlueprintCallable, Category = "Goombanics|Projectile")
	void Initialize(float InDamage, float InSplashRadius, float InSplashDamage, float InSpeed);

protected:
	UFUNCTION()
	void OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);

	void Explode(const FVector& Location);
	void ApplySplashDamage(const FVector& Location);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goombanics|Components")
	TObjectPtr<USphereComponent> CollisionComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goombanics|Components")
	TObjectPtr<UStaticMeshComponent> MeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Goombanics|Components")
	TObjectPtr<UProjectileMovementComponent> ProjectileMovement;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Projectile")
	float Damage = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Projectile")
	float SplashRadius = 400.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Projectile")
	float SplashDamage = 75.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Goombanics|Projectile")
	float LifeSpan = 10.0f;
};
