// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsProjectile.h"
#include "Goombanics/Monster/GoombanicsMonsterInterface.h"
#include "Goombanics/Destruction/GoombanicsBreakableActor.h"
#include "Goombanics/Goombanics.h"
#include "Components/SphereComponent.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"

AGoombanicsProjectile::AGoombanicsProjectile()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("CollisionComponent"));
	CollisionComponent->InitSphereRadius(15.0f);
	CollisionComponent->SetCollisionProfileName(TEXT("Projectile"));
	CollisionComponent->SetGenerateOverlapEvents(false);
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovement"));
	ProjectileMovement->UpdatedComponent = CollisionComponent;
	ProjectileMovement->InitialSpeed = 2000.0f;
	ProjectileMovement->MaxSpeed = 2000.0f;
	ProjectileMovement->bRotationFollowsVelocity = true;
	ProjectileMovement->bShouldBounce = false;
	ProjectileMovement->ProjectileGravityScale = 0.1f;

	bReplicates = true;
}

void AGoombanicsProjectile::BeginPlay()
{
	Super::BeginPlay();

	CollisionComponent->OnComponentHit.AddDynamic(this, &AGoombanicsProjectile::OnHit);
	SetLifeSpan(LifeSpan);
}

void AGoombanicsProjectile::Initialize(float InDamage, float InSplashRadius, float InSplashDamage, float InSpeed)
{
	Damage = InDamage;
	SplashRadius = InSplashRadius;
	SplashDamage = InSplashDamage;

	if (ProjectileMovement)
	{
		ProjectileMovement->InitialSpeed = InSpeed;
		ProjectileMovement->MaxSpeed = InSpeed;
		ProjectileMovement->Velocity = GetActorForwardVector() * InSpeed;
	}
}

void AGoombanicsProjectile::OnHit(UPrimitiveComponent* HitComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit)
{
	if (OtherActor && OtherActor != GetOwner())
	{
		Explode(Hit.ImpactPoint);
	}
}

void AGoombanicsProjectile::Explode(const FVector& Location)
{
	ApplySplashDamage(Location);
	Destroy();
}

void AGoombanicsProjectile::ApplySplashDamage(const FVector& Location)
{
	AController* InstigatorController = nullptr;
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		InstigatorController = OwnerPawn->GetController();
	}

	TArray<AActor*> IgnoredActors;
	IgnoredActors.Add(this);
	IgnoredActors.Add(GetOwner());

	TArray<FHitResult> HitResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(SplashRadius);
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActors(IgnoredActors);

	if (GetWorld()->SweepMultiByChannel(HitResults, Location, Location, FQuat::Identity, ECC_Visibility, Sphere, QueryParams))
	{
		TSet<AActor*> ProcessedActors;

		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && !ProcessedActors.Contains(HitActor))
			{
				ProcessedActors.Add(HitActor);

				float Distance = FVector::Dist(Location, HitActor->GetActorLocation());
				float DamageScale = 1.0f - (Distance / SplashRadius);
				float ActualDamage = SplashDamage * FMath::Max(0.0f, DamageScale);

				if (HitActor->Implements<UGoombanicsMonsterInterface>())
				{
					IGoombanicsMonsterInterface::Execute_ApplyDamageToMonster(HitActor, ActualDamage, InstigatorController, this);
				}
				else if (AGoombanicsBreakableActor* Breakable = Cast<AGoombanicsBreakableActor>(HitActor))
				{
					Breakable->Break(InstigatorController ? InstigatorController->GetPlayerState<APlayerState>() : nullptr);
				}
				else
				{
					FDamageEvent DamageEvent;
					HitActor->TakeDamage(ActualDamage, DamageEvent, InstigatorController, this);
				}
			}
		}
	}
}
