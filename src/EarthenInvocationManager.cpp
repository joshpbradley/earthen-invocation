// Copyright (C) I N F I N I T Y 27 LTD. All rights reserved. 

#include "EarthenInvocationManager.h"

#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "../../../Plugins/AscentCombatFramework/Source/AscentCombatFramework/Public/Components/ACFDamageHandlerComponent.h"
#include "Actors/ACFCharacter.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/World.h"

#include "../DamageTypes/PoiseDamageType.h"
#include "../DamageTypes/EarthDamageType.h"

/**
 * Spawns a given number of projectiles in an enclosed box region. It also sets the
 * rotation angle of each particle relative to the central direction vector
 * and the spread value.
 */
void AEarthenInvocationManager::SpawnProjectiles(TSubclassOf<AActor> _projectile, int _numberOfProjectiles,
	FVector _origin, FVector _extent, FVector _centralDirectionUnit, float _spread, UMaterialInterface* _material,
	UMaterialInterface* _overlayMaterial, FVector _scale)
{
	projectiles.Reserve(_numberOfProjectiles);

	for (size_t i = 0; i < _numberOfProjectiles; i++)
	{
		// Determines how much vertical spread occurs in a spell cast. The larger the number, the lower the spread.
		const double VERTICAL_SPREAD_MODIFIER = 450;

		// Calculates the initial location and rotation properties of the projectile transform.
		FVector location = UKismetMathLibrary::RandomPointInBoundingBox(_origin, _extent);
		FRotator pitchRotation = UKismetMathLibrary::MakeRotFromX(location - (_origin - (_centralDirectionUnit * (_extent.X + VERTICAL_SPREAD_MODIFIER))));
		FRotator yawRotation = UKismetMathLibrary::MakeRotFromX(location - (_origin - (_centralDirectionUnit * (_extent.X + _spread))));

		// Sets the final rotations, including a randomised roll angle [0-359].
		FRotator rotation(pitchRotation.Pitch, yawRotation.Yaw, UKismetMathLibrary::RandomInteger(360));

		// Sets miscellaneous spawn paramaters for the projectile.
		FActorSpawnParameters spawnParams;
		spawnParams.Instigator = GetInstigator();
		spawnParams.TransformScaleMethod = ESpawnActorScaleMethod::OverrideRootScale;

		// Spawns a new projectile.
		auto projectile = GetWorld()->SpawnActor<AActor>(_projectile->GetDefaultObject()->GetClass(), location, rotation, spawnParams);
		projectile->SetActorScale3D(_scale * UKismetMathLibrary::RandomFloatInRange(0.5, 1.5));

		auto mesh = projectile->GetComponentByClass<UStaticMeshComponent>();

		// Applies material(s) to the projectile.
		mesh->SetMaterial(0, _material);
		if (_overlayMaterial)
		{
			mesh->SetOverlayMaterial(_overlayMaterial);
		}

		// Adds the projectile to the projectiles array, marking it as active.
		projectiles.Add(projectile, true);
	}
}

/**
 * Determines whether the two Actors are enemies of each other.
 */
bool AEarthenInvocationManager::AreActorsEnemies(AActor* _actorOne, AActor* _actorTwo)
{
	if (_actorOne && _actorTwo)
	{
		auto actorOneCast = Cast<AACFCharacter>(_actorOne);
		auto actorTwoCast = Cast<AACFCharacter>(_actorTwo);

		if (actorOneCast && actorTwoCast)
		{
			return actorOneCast->IsMyEnemy(actorTwoCast);
		}
	}

	return false;
}

/**
 * Damages the target with poise and earth damage. This occurs if the target actor is considered to be the enemy character of the casting actor.
 * 
 * Successive hits on a given enemy by projectiles managed by the same manager instance will result in damage being scaled down.
 */
bool AEarthenInvocationManager::ApplyDamage(AActor* _target, float _earthDamage, float _poiseDamage, float _damageReduceScale,
	TSubclassOf<UEarthDamageType> _earthDamageClass, TSubclassOf<UPoiseDamageType> _poiseDamageClass)
{
	/*
	 * Checks the target has an attached ACFDamageComponent and that the target can receive damage.
	 */
	if (_target && _target->GetComponentByClass<UACFDamageHandlerComponent>() && _target->CanBeDamaged())
	{
		// Determines whether the enemy has already been hit by a projectile managed by this manager instance.
		bool hasBeenHit = (enemiesHit.Find(_target) != INDEX_NONE);

		// Applies the damage, scaling the damage amount down if an enemy has already been hit.
		UGameplayStatics::ApplyDamage(_target,
			hasBeenHit ? (_earthDamage / _damageReduceScale) : _earthDamage,
			GetInstigatorController(), GetInstigator(), _earthDamageClass);

		if (!hasBeenHit)
		{
			/*
			 * Damages an enemy only once per cast with poise damage.
			 * Testing has shown that multiple bursts of damage application are required to remove multiple poise segments.
			 */

			// The amount of poise represented by a single segment in a character's poise bar.
			const int DAMAGE_PER_POISE_SEGMENT = 5;
			size_t poiseDamageSteps = UKismetMathLibrary::FCeil(_poiseDamage / DAMAGE_PER_POISE_SEGMENT);

			for (size_t i = 0; i < poiseDamageSteps; i++)
			{
				UGameplayStatics::ApplyDamage(_target, _poiseDamage / poiseDamageSteps, GetInstigatorController(), GetInstigator(), _poiseDamageClass);
			}

			enemiesHit.Add(_target);
		}

		return true;
	}

	return false;
}