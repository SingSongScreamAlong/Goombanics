// Copyright Epic Games, Inc. All Rights Reserved.

#include "GoombanicsWeaponComponent.h"
#include "GoombanicsProjectile.h"
#include "Goombanics/Monster/GoombanicsMonsterBase.h"
#include "Goombanics/Monster/GoombanicsMonsterInterface.h"
#include "Goombanics/Destruction/GoombanicsBreakableActor.h"
#include "Goombanics/Core/GoombanicsTypes.h"
#include "Goombanics/Goombanics.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/DamageEvents.h"
#include "GameFramework/Character.h"
#include "GameFramework/PlayerController.h"

FGoombanicsWeaponStats UGoombanicsWeaponComponent::EmptyWeaponStats;

UGoombanicsWeaponComponent::UGoombanicsWeaponComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	FGoombanicsWeaponStats AssaultRifle;
	AssaultRifle.WeaponName = FName("Assault Rifle");
	AssaultRifle.FireMode = EWeaponFireMode::Hitscan;
	AssaultRifle.Damage = 25.0f;
	AssaultRifle.FireRate = 10.0f;
	AssaultRifle.Range = 10000.0f;
	AssaultRifle.AmmoCapacity = 30;
	AssaultRifle.ReloadTime = 2.0f;
	Weapons.Add(AssaultRifle);

	FGoombanicsWeaponStats RocketLauncher;
	RocketLauncher.WeaponName = FName("Rocket Launcher");
	RocketLauncher.FireMode = EWeaponFireMode::Projectile;
	RocketLauncher.Damage = 100.0f;
	RocketLauncher.FireRate = 1.0f;
	RocketLauncher.Range = 15000.0f;
	RocketLauncher.SplashRadius = 400.0f;
	RocketLauncher.SplashDamage = 75.0f;
	RocketLauncher.ProjectileSpeed = 2000.0f;
	RocketLauncher.AmmoCapacity = 4;
	RocketLauncher.ReloadTime = 3.0f;
	Weapons.Add(RocketLauncher);
}

void UGoombanicsWeaponComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Weapons.Num() > 0)
	{
		CurrentAmmo = Weapons[CurrentWeaponIndex].AmmoCapacity;
	}
}

void UGoombanicsWeaponComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (FireCooldown > 0.0f)
	{
		FireCooldown -= DeltaTime;
	}

	if (bIsReloading)
	{
		ReloadTimeRemaining -= DeltaTime;
		if (ReloadTimeRemaining <= 0.0f)
		{
			bIsReloading = false;
			if (Weapons.IsValidIndex(CurrentWeaponIndex))
			{
				CurrentAmmo = Weapons[CurrentWeaponIndex].AmmoCapacity;
			}
			OnReloadFinished.Broadcast();
		}
	}

	if (bWantsToFire && !bIsReloading && FireCooldown <= 0.0f)
	{
		Fire();
	}
}

void UGoombanicsWeaponComponent::StartFire()
{
	bWantsToFire = true;
}

void UGoombanicsWeaponComponent::StopFire()
{
	bWantsToFire = false;
}

void UGoombanicsWeaponComponent::SwitchToNextWeapon()
{
	if (Weapons.Num() <= 1)
	{
		return;
	}

	int32 NewIndex = (CurrentWeaponIndex + 1) % Weapons.Num();
	SwitchToWeapon(NewIndex);
}

void UGoombanicsWeaponComponent::SwitchToWeapon(int32 WeaponIndex)
{
	if (!Weapons.IsValidIndex(WeaponIndex) || WeaponIndex == CurrentWeaponIndex)
	{
		return;
	}

	CurrentWeaponIndex = WeaponIndex;
	CurrentAmmo = Weapons[CurrentWeaponIndex].AmmoCapacity;
	bIsReloading = false;
	FireCooldown = 0.0f;

	OnWeaponSwitched.Broadcast(CurrentWeaponIndex);
}

void UGoombanicsWeaponComponent::StartReload()
{
	if (bIsReloading || !Weapons.IsValidIndex(CurrentWeaponIndex))
	{
		return;
	}

	if (CurrentAmmo >= Weapons[CurrentWeaponIndex].AmmoCapacity)
	{
		return;
	}

	bIsReloading = true;
	ReloadTimeRemaining = Weapons[CurrentWeaponIndex].ReloadTime;
	OnReloadStarted.Broadcast(ReloadTimeRemaining);
}

const FGoombanicsWeaponStats& UGoombanicsWeaponComponent::GetCurrentWeaponStats() const
{
	if (Weapons.IsValidIndex(CurrentWeaponIndex))
	{
		return Weapons[CurrentWeaponIndex];
	}
	return EmptyWeaponStats;
}

void UGoombanicsWeaponComponent::Fire()
{
	if (!Weapons.IsValidIndex(CurrentWeaponIndex))
	{
		return;
	}

	if (CurrentAmmo <= 0)
	{
		StartReload();
		return;
	}

	const FGoombanicsWeaponStats& Stats = Weapons[CurrentWeaponIndex];

	if (Stats.FireMode == EWeaponFireMode::Hitscan)
	{
		FireHitscan();
	}
	else
	{
		FireProjectile();
	}

	CurrentAmmo--;
	FireCooldown = 1.0f / Stats.FireRate;

	OnWeaponFired.Broadcast(CurrentWeaponIndex, CurrentAmmo);
}

void UGoombanicsWeaponComponent::FireHitscan()
{
	const FGoombanicsWeaponStats& Stats = Weapons[CurrentWeaponIndex];

	FVector Start = GetMuzzleLocation();
	FVector Direction = GetAimDirection();
	FVector End = Start + Direction * Stats.Range;

	FHitResult HitResult;
	FCollisionQueryParams QueryParams;
	QueryParams.AddIgnoredActor(GetOwner());

	if (GetWorld()->LineTraceSingleByChannel(HitResult, Start, End, ECC_Visibility, QueryParams))
	{
		ProcessHit(HitResult, Stats.Damage);
	}
}

void UGoombanicsWeaponComponent::FireProjectile()
{
	const FGoombanicsWeaponStats& Stats = Weapons[CurrentWeaponIndex];

	if (!Stats.ProjectileClass)
	{
		UE_LOG(LogGoombanics, Warning, TEXT("No projectile class set for weapon %s"), *Stats.WeaponName.ToString());
		return;
	}

	FVector SpawnLocation = GetMuzzleLocation();
	FRotator SpawnRotation = GetAimDirection().Rotation();

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = GetOwner();
	SpawnParams.Instigator = Cast<APawn>(GetOwner());

	AGoombanicsProjectile* Projectile = GetWorld()->SpawnActor<AGoombanicsProjectile>(
		Stats.ProjectileClass, SpawnLocation, SpawnRotation, SpawnParams);

	if (Projectile)
	{
		Projectile->Initialize(Stats.Damage, Stats.SplashRadius, Stats.SplashDamage, Stats.ProjectileSpeed);
	}
}

void UGoombanicsWeaponComponent::ProcessHit(const FHitResult& HitResult, float Damage)
{
	AActor* HitActor = HitResult.GetActor();
	if (!HitActor)
	{
		return;
	}

	AController* InstigatorController = nullptr;
	if (APawn* OwnerPawn = Cast<APawn>(GetOwner()))
	{
		InstigatorController = OwnerPawn->GetController();
	}

	if (HitActor->Implements<UGoombanicsMonsterInterface>())
	{
		UPrimitiveComponent* HitComponent = HitResult.GetComponent();
		EGoombanicsWeakPointType WeakPointType = EGoombanicsWeakPointType::None;

		if (HitComponent)
		{
			if (HitComponent->ComponentHasTag(FName("WeakPoint_LeftLeg")))
			{
				WeakPointType = EGoombanicsWeakPointType::LeftLeg;
			}
			else if (HitComponent->ComponentHasTag(FName("WeakPoint_RightLeg")))
			{
				WeakPointType = EGoombanicsWeakPointType::RightLeg;
			}
			else if (HitComponent->ComponentHasTag(FName("WeakPoint_Head")))
			{
				WeakPointType = EGoombanicsWeakPointType::Head;
			}
		}

		if (WeakPointType != EGoombanicsWeakPointType::None)
		{
			IGoombanicsMonsterInterface::Execute_ApplyDamageToWeakPoint(HitActor, WeakPointType, Damage, InstigatorController);
		}
		else
		{
			IGoombanicsMonsterInterface::Execute_ApplyDamageToMonster(HitActor, Damage, InstigatorController, GetOwner());
		}
	}
	else if (AGoombanicsBreakableActor* Breakable = Cast<AGoombanicsBreakableActor>(HitActor))
	{
		Breakable->Break(InstigatorController ? InstigatorController->GetPlayerState<APlayerState>() : nullptr);
	}
	else
	{
		FDamageEvent DamageEvent;
		HitActor->TakeDamage(Damage, DamageEvent, InstigatorController, GetOwner());
	}
}

void UGoombanicsWeaponComponent::ApplySplashDamage(const FVector& Location, float Radius, float Damage)
{
	TArray<FHitResult> HitResults;
	FCollisionShape Sphere = FCollisionShape::MakeSphere(Radius);

	if (GetWorld()->SweepMultiByChannel(HitResults, Location, Location, FQuat::Identity, ECC_Visibility, Sphere))
	{
		TSet<AActor*> ProcessedActors;

		for (const FHitResult& Hit : HitResults)
		{
			AActor* HitActor = Hit.GetActor();
			if (HitActor && !ProcessedActors.Contains(HitActor) && HitActor != GetOwner())
			{
				ProcessedActors.Add(HitActor);

				float Distance = FVector::Dist(Location, HitActor->GetActorLocation());
				float DamageScale = 1.0f - (Distance / Radius);
				float ActualDamage = Damage * FMath::Max(0.0f, DamageScale);

				ProcessHit(Hit, ActualDamage);
			}
		}
	}
}

FVector UGoombanicsWeaponComponent::GetMuzzleLocation() const
{
	if (AActor* Owner = GetOwner())
	{
		if (ACharacter* Character = Cast<ACharacter>(Owner))
		{
			return Character->GetActorLocation() + Character->GetActorForwardVector() * 100.0f + FVector(0, 0, 50.0f);
		}
		return Owner->GetActorLocation();
	}
	return FVector::ZeroVector;
}

FVector UGoombanicsWeaponComponent::GetAimDirection() const
{
	if (AActor* Owner = GetOwner())
	{
		if (APawn* Pawn = Cast<APawn>(Owner))
		{
			if (APlayerController* PC = Cast<APlayerController>(Pawn->GetController()))
			{
				FVector CameraLocation;
				FRotator CameraRotation;
				PC->GetPlayerViewPoint(CameraLocation, CameraRotation);
				return CameraRotation.Vector();
			}
		}
		return Owner->GetActorForwardVector();
	}
	return FVector::ForwardVector;
}
