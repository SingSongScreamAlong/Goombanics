// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsMonsterBase.h"
#include "Goombanics/Core/GoombanicsGameState.h"
#include "Goombanics/Player/GoombanicsPlayerState.h"
#include "Goombanics/Goombanics.h"
#include "Components/CapsuleComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "Kismet/GameplayStatics.h"
#include "AIController.h"

AGoombanicsMonsterBase::AGoombanicsMonsterBase()
{
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>(TEXT("CapsuleComponent"));
	CapsuleComponent->InitCapsuleSize(200.0f, 500.0f);
	CapsuleComponent->SetCollisionProfileName(TEXT("Pawn"));
	RootComponent = CapsuleComponent;

	MeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);

	MovementComponent = CreateDefaultSubobject<UFloatingPawnMovement>(TEXT("MovementComponent"));
	MovementComponent->MaxSpeed = 400.0f;

	WeakPoints.Add(FGoombanicsWeakPointState{EGoombanicsWeakPointType::LeftLeg, 100.0f, 100.0f, false});
	WeakPoints.Add(FGoombanicsWeakPointState{EGoombanicsWeakPointType::RightLeg, 100.0f, 100.0f, false});
	WeakPoints.Add(FGoombanicsWeakPointState{EGoombanicsWeakPointType::Head, 200.0f, 200.0f, false});

	CurrentHealth = MaxHealth;
}

void AGoombanicsMonsterBase::BeginPlay()
{
	Super::BeginPlay();
	CurrentHealth = MaxHealth;
	UpdateGameState();
}

void AGoombanicsMonsterBase::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	UpdateStagger(DeltaTime);
}

void AGoombanicsMonsterBase::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	if (NewController->IsA<AAIController>())
	{
		bIsControlledByPlayer = false;
		Execute_OnPossessedByAI(this, NewController);
	}
	else
	{
		bIsControlledByPlayer = true;
		Execute_OnPossessedByPlayer(this, NewController);
	}
}

void AGoombanicsMonsterBase::UnPossessed()
{
	Super::UnPossessed();
	bIsControlledByPlayer = false;
}

float AGoombanicsMonsterBase::GetMonsterHealthPercent_Implementation() const
{
	return MaxHealth > 0.0f ? CurrentHealth / MaxHealth : 0.0f;
}

void AGoombanicsMonsterBase::ApplyDamageToMonster_Implementation(float Damage, AController* Instigator, AActor* DamageCauser)
{
	if (bIsDead)
	{
		return;
	}

	CurrentHealth = FMath::Max(0.0f, CurrentHealth - Damage);

	if (Instigator)
	{
		if (AGoombanicsPlayerState* PS = Instigator->GetPlayerState<AGoombanicsPlayerState>())
		{
			PS->AddKaijuDamage(Damage);
		}
	}

	UpdateGameState();

	if (CurrentHealth <= 0.0f)
	{
		Die(Instigator);
	}

	UE_LOG(LogGoombanics, Verbose, TEXT("Monster took %.1f damage, health: %.1f/%.1f"), Damage, CurrentHealth, MaxHealth);
}

void AGoombanicsMonsterBase::ApplyDamageToWeakPoint_Implementation(EGoombanicsWeakPointType WeakPointType, float Damage, AController* Instigator)
{
	if (bIsDead)
	{
		return;
	}

	for (FGoombanicsWeakPointState& WP : WeakPoints)
	{
		if (WP.WeakPointType == WeakPointType && !WP.bIsDestroyed)
		{
			if (WeakPointType == EGoombanicsWeakPointType::Head && bIsStaggered)
			{
				Damage *= HeadDamageMultiplierDuringStagger;
			}

			WP.CurrentHealth = FMath::Max(0.0f, WP.CurrentHealth - Damage);

			Execute_ApplyDamageToMonster(this, Damage * 0.5f, Instigator, nullptr);

			if (WP.CurrentHealth <= 0.0f)
			{
				WP.bIsDestroyed = true;
				OnWeakPointDestroyed(WeakPointType, Instigator);
			}

			if (AGoombanicsGameState* GS = Cast<AGoombanicsGameState>(UGameplayStatics::GetGameState(GetWorld())))
			{
				GS->UpdateWeakPointState(WeakPointType, WP.CurrentHealth, WP.bIsDestroyed, 
					Instigator ? Instigator->GetPlayerState<APlayerState>() : nullptr);
			}

			break;
		}
	}

	CheckStaggerConditions();
}

bool AGoombanicsMonsterBase::IsMonsterStaggered_Implementation() const
{
	return bIsStaggered;
}

void AGoombanicsMonsterBase::TriggerStagger_Implementation()
{
	if (!bIsStaggered)
	{
		bIsStaggered = true;
		StaggerTimeRemaining = StaggerDuration;

		if (AGoombanicsGameState* GS = Cast<AGoombanicsGameState>(UGameplayStatics::GetGameState(GetWorld())))
		{
			GS->SetKaijuStaggered(true);
		}

		UE_LOG(LogGoombanics, Log, TEXT("Monster staggered!"));
	}
}

void AGoombanicsMonsterBase::EndStagger_Implementation()
{
	if (bIsStaggered)
	{
		bIsStaggered = false;
		StaggerTimeRemaining = 0.0f;

		if (AGoombanicsGameState* GS = Cast<AGoombanicsGameState>(UGameplayStatics::GetGameState(GetWorld())))
		{
			GS->SetKaijuStaggered(false);
		}

		UE_LOG(LogGoombanics, Log, TEXT("Monster recovered from stagger"));
	}
}

TArray<FGoombanicsWeakPointState> AGoombanicsMonsterBase::GetWeakPointStates_Implementation() const
{
	return WeakPoints;
}

bool AGoombanicsMonsterBase::IsWeakPointExposed_Implementation(EGoombanicsWeakPointType WeakPointType) const
{
	if (WeakPointType == EGoombanicsWeakPointType::Head)
	{
		return bIsStaggered;
	}
	return true;
}

void AGoombanicsMonsterBase::OnPossessedByPlayer_Implementation(AController* NewController)
{
	UE_LOG(LogGoombanics, Log, TEXT("Monster possessed by player"));
}

void AGoombanicsMonsterBase::OnPossessedByAI_Implementation(AController* NewController)
{
	UE_LOG(LogGoombanics, Log, TEXT("Monster possessed by AI"));
}

void AGoombanicsMonsterBase::Die(AController* Killer)
{
	if (bIsDead)
	{
		return;
	}

	bIsDead = true;

	if (Killer)
	{
		if (AGoombanicsPlayerState* PS = Killer->GetPlayerState<AGoombanicsPlayerState>())
		{
			PS->IncrementFinalBlowCount();
		}
	}

	OnDeath(Killer);
	UpdateGameState();

	UE_LOG(LogGoombanics, Log, TEXT("Monster died"));
}

void AGoombanicsMonsterBase::UpdateStagger(float DeltaTime)
{
	if (bIsStaggered)
	{
		StaggerTimeRemaining -= DeltaTime;
		if (StaggerTimeRemaining <= 0.0f)
		{
			Execute_EndStagger(this);
		}
	}
}

void AGoombanicsMonsterBase::CheckStaggerConditions()
{
	bool bLeftLegDestroyed = false;
	bool bRightLegDestroyed = false;

	for (const FGoombanicsWeakPointState& WP : WeakPoints)
	{
		if (WP.WeakPointType == EGoombanicsWeakPointType::LeftLeg && WP.bIsDestroyed)
		{
			bLeftLegDestroyed = true;
		}
		if (WP.WeakPointType == EGoombanicsWeakPointType::RightLeg && WP.bIsDestroyed)
		{
			bRightLegDestroyed = true;
		}
	}

	if (bLeftLegDestroyed && bRightLegDestroyed && !bIsStaggered)
	{
		Execute_TriggerStagger(this);
	}
}

void AGoombanicsMonsterBase::OnWeakPointDestroyed(EGoombanicsWeakPointType WeakPointType, AController* Destroyer)
{
	UE_LOG(LogGoombanics, Log, TEXT("Weak point destroyed: %d"), static_cast<int32>(WeakPointType));
}

void AGoombanicsMonsterBase::OnDeath(AController* Killer)
{
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);
}

void AGoombanicsMonsterBase::UpdateGameState()
{
	if (AGoombanicsGameState* GS = Cast<AGoombanicsGameState>(UGameplayStatics::GetGameState(GetWorld())))
	{
		GS->SetKaijuHealthPercent(GetMonsterHealthPercent_Implementation());
		GS->SetWeakPointStates(WeakPoints);
	}
}
