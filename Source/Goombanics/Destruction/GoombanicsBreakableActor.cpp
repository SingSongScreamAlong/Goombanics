// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsBreakableActor.h"
#include "Goombanics/Core/GoombanicsGameState.h"
#include "Goombanics/Goombanics.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Kismet/GameplayStatics.h"

AGoombanicsBreakableActor::AGoombanicsBreakableActor()
{
	PrimaryActorTick.bCanEverTick = false;

	CollisionComponent = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionComponent"));
	CollisionComponent->SetBoxExtent(FVector(50.0f, 50.0f, 100.0f));
	CollisionComponent->SetCollisionProfileName(TEXT("BlockAll"));
	RootComponent = CollisionComponent;

	MeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("MeshComponent"));
	MeshComponent->SetupAttachment(RootComponent);
	MeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bReplicates = true;
}

void AGoombanicsBreakableActor::BeginPlay()
{
	Super::BeginPlay();
}

void AGoombanicsBreakableActor::Break(APlayerState* Instigator)
{
	if (bIsBroken)
	{
		return;
	}

	bIsBroken = true;
	OnBroken(Instigator);
}

void AGoombanicsBreakableActor::OnBroken(APlayerState* Instigator)
{
	if (bContributesToDestructionMeter)
	{
		if (AGoombanicsGameState* GS = Cast<AGoombanicsGameState>(UGameplayStatics::GetGameState(GetWorld())))
		{
			GS->AddDestructionValue(DestructionValue, Instigator);
		}
	}

	OnBreakableDestroyed.Broadcast(this, Instigator);

	if (BrokenMesh)
	{
		MeshComponent->SetStaticMesh(BrokenMesh);
	}
	else
	{
		SetActorHiddenInGame(true);
		SetActorEnableCollision(false);
	}

	UE_LOG(LogGoombanics, Verbose, TEXT("Breakable destroyed: %s, value: %.1f"), *GetName(), DestructionValue);
}
