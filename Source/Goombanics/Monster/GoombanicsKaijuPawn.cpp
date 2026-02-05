// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsKaijuPawn.h"
#include "Goombanics/Player/GoombanicsCharacter.h"
#include "Goombanics/Destruction/GoombanicsBreakableActor.h"
#include "Goombanics/Goombanics.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Engine/DamageEvents.h"

AGoombanicsKaijuPawn::AGoombanicsKaijuPawn()
{
	MaxHealth = 5000.0f;
	CurrentHealth = MaxHealth;

	LeftLegHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("LeftLegHitbox"));
	LeftLegHitbox->SetupAttachment(RootComponent);
	LeftLegHitbox->SetBoxExtent(FVector(50.0f, 50.0f, 150.0f));
	LeftLegHitbox->SetRelativeLocation(FVector(0.0f, -100.0f, -200.0f));
	LeftLegHitbox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	LeftLegHitbox->ComponentTags.Add(FName("WeakPoint_LeftLeg"));

	RightLegHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("RightLegHitbox"));
	RightLegHitbox->SetupAttachment(RootComponent);
	RightLegHitbox->SetBoxExtent(FVector(50.0f, 50.0f, 150.0f));
	RightLegHitbox->SetRelativeLocation(FVector(0.0f, 100.0f, -200.0f));
	RightLegHitbox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	RightLegHitbox->ComponentTags.Add(FName("WeakPoint_RightLeg"));

	HeadHitbox = CreateDefaultSubobject<UBoxComponent>(TEXT("HeadHitbox"));
	HeadHitbox->SetupAttachment(RootComponent);
	HeadHitbox->SetBoxExtent(FVector(75.0f, 75.0f, 75.0f));
	HeadHitbox->SetRelativeLocation(FVector(0.0f, 0.0f, 400.0f));
	HeadHitbox->SetCollisionProfileName(TEXT("OverlapAllDynamic"));
	HeadHitbox->ComponentTags.Add(FName("WeakPoint_Head"));
}

void AGoombanicsKaijuPawn::BeginPlay()
{
	Super::BeginPlay();
	CurrentAIState = EKaijuAIState::Idle;
}

void AGoombanicsKaijuPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (bAIEnabled && !bIsControlledByPlayer && !bIsDead)
	{
		UpdateAI(DeltaTime);
	}

	if (AttackCooldownRemaining > 0.0f)
	{
		AttackCooldownRemaining -= DeltaTime;
	}

	if (bIsAttacking)
	{
		AttackTimeRemaining -= DeltaTime;
		if (AttackTimeRemaining <= 0.0f)
		{
			bIsAttacking = false;
			CurrentAIState = EKaijuAIState::Pursuing;
		}
	}
}

void AGoombanicsKaijuPawn::TriggerStagger_Implementation()
{
	Super::TriggerStagger_Implementation();
	CurrentAIState = EKaijuAIState::Staggered;
	bIsAttacking = false;
}

void AGoombanicsKaijuPawn::EndStagger_Implementation()
{
	Super::EndStagger_Implementation();
	if (!bIsDead)
	{
		CurrentAIState = EKaijuAIState::Pursuing;
	}
}

void AGoombanicsKaijuPawn::SetAIEnabled(bool bEnabled)
{
	bAIEnabled = bEnabled;
	if (!bEnabled)
	{
		CurrentAIState = EKaijuAIState::Idle;
	}
}

void AGoombanicsKaijuPawn::PerformStompAttack()
{
	if (bIsAttacking || AttackCooldownRemaining > 0.0f || bIsStaggered)
	{
		return;
	}

	bIsAttacking = true;
	AttackTimeRemaining = AttackDuration;
	AttackCooldownRemaining = AttackCooldown;
	CurrentAIState = EKaijuAIState::Attacking;

	FVector AttackCenter = GetActorLocation() + FVector(0.0f, 0.0f, -200.0f);
	ApplyAttackDamage(AttackCenter, StompRadius, StompDamage);
	DamageNearbyDestructibles(AttackCenter, DestructionRadius);

	UE_LOG(LogGoombanics, Verbose, TEXT("Kaiju stomp attack"));
}

void AGoombanicsKaijuPawn::PerformSweepAttack()
{
	if (bIsAttacking || AttackCooldownRemaining > 0.0f || bIsStaggered)
	{
		return;
	}

	bIsAttacking = true;
	AttackTimeRemaining = AttackDuration;
	AttackCooldownRemaining = AttackCooldown;
	CurrentAIState = EKaijuAIState::Attacking;

	FVector AttackCenter = GetActorLocation() + GetActorForwardVector() * 300.0f;
	ApplyAttackDamage(AttackCenter, SweepRadius, SweepDamage);
	DamageNearbyDestructibles(AttackCenter, DestructionRadius);

	UE_LOG(LogGoombanics, Verbose, TEXT("Kaiju sweep attack"));
}

void AGoombanicsKaijuPawn::UpdateAI(float DeltaTime)
{
	if (CurrentAIState == EKaijuAIState::Dead || CurrentAIState == EKaijuAIState::Staggered)
	{
		return;
	}

	TargetUpdateTimer -= DeltaTime;
	if (TargetUpdateTimer <= 0.0f)
	{
		CurrentTarget = FindNearestPlayer();
		TargetUpdateTimer = TargetUpdateInterval;
	}

	if (!CurrentTarget.IsValid())
	{
		CurrentAIState = EKaijuAIState::Idle;
		return;
	}

	if (CurrentAIState == EKaijuAIState::Attacking)
	{
		UpdateAttack(DeltaTime);
		return;
	}

	float DistanceToTarget = FVector::Dist(GetActorLocation(), CurrentTarget->GetActorLocation());

	if (DistanceToTarget <= AttackRange && AttackCooldownRemaining <= 0.0f)
	{
		SelectAttack();
	}
	else
	{
		CurrentAIState = EKaijuAIState::Pursuing;
		UpdatePursuit(DeltaTime);
	}
}

void AGoombanicsKaijuPawn::UpdatePursuit(float DeltaTime)
{
	if (!CurrentTarget.IsValid())
	{
		return;
	}

	FVector Direction = (CurrentTarget->GetActorLocation() - GetActorLocation()).GetSafeNormal2D();
	FVector NewLocation = GetActorLocation() + Direction * PursuitSpeed * DeltaTime;
	SetActorLocation(NewLocation);

	FRotator TargetRotation = Direction.Rotation();
	SetActorRotation(FMath::RInterpTo(GetActorRotation(), TargetRotation, DeltaTime, 2.0f));

	DamageNearbyDestructibles(GetActorLocation(), DestructionRadius * 0.5f);
}

void AGoombanicsKaijuPawn::UpdateAttack(float DeltaTime)
{
}

AActor* AGoombanicsKaijuPawn::FindNearestPlayer() const
{
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoombanicsCharacter::StaticClass(), Players);

	AActor* NearestPlayer = nullptr;
	float NearestDistance = TNumericLimits<float>::Max();

	for (AActor* Player : Players)
	{
		AGoombanicsCharacter* Character = Cast<AGoombanicsCharacter>(Player);
		if (Character && Character->IsAlive())
		{
			float Distance = FVector::Dist(GetActorLocation(), Player->GetActorLocation());
			if (Distance < NearestDistance)
			{
				NearestDistance = Distance;
				NearestPlayer = Player;
			}
		}
	}

	return NearestPlayer;
}

void AGoombanicsKaijuPawn::SelectAttack()
{
	if (FMath::RandBool())
	{
		PerformStompAttack();
	}
	else
	{
		PerformSweepAttack();
	}
}

void AGoombanicsKaijuPawn::ApplyAttackDamage(const FVector& Center, float Radius, float Damage)
{
	TArray<AActor*> Players;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoombanicsCharacter::StaticClass(), Players);

	for (AActor* Player : Players)
	{
		float Distance = FVector::Dist(Center, Player->GetActorLocation());
		if (Distance <= Radius)
		{
			float DamageScale = 1.0f - (Distance / Radius);
			float ActualDamage = Damage * DamageScale;

			FDamageEvent DamageEvent;
			Player->TakeDamage(ActualDamage, DamageEvent, GetController(), this);
		}
	}
}

void AGoombanicsKaijuPawn::DamageNearbyDestructibles(const FVector& Center, float Radius)
{
	TArray<AActor*> Destructibles;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AGoombanicsBreakableActor::StaticClass(), Destructibles);

	for (AActor* Actor : Destructibles)
	{
		float Distance = FVector::Dist(Center, Actor->GetActorLocation());
		if (Distance <= Radius)
		{
			if (AGoombanicsBreakableActor* Breakable = Cast<AGoombanicsBreakableActor>(Actor))
			{
				Breakable->Break(nullptr);
			}
		}
	}
}
