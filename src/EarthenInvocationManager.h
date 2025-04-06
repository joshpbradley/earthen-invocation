// Copyright (C) I N F I N I T Y 27 LTD. All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "EarthenInvocationManager.generated.h"

UCLASS()
class SAMSARA_API AEarthenInvocationManager : public AActor
{
	GENERATED_BODY()

protected:
	// Stores the Earthen Invocation mantra projectiles and whether they are active. A hit or elapsed lifetime deactivates the projectile.
	UPROPERTY(BlueprintReadWrite, VisibleAnywhere)
	TMap<AActor*, bool> projectiles = {};
	// Stores the enemies that have been hit by projectiles managed by this manager instance.
	UPROPERTY(BlueprintReadOnly, VisibleAnywhere)
	TArray<AActor*> enemiesHit = {};

protected:
	/**
	 * Spawns a given number of projectiles in an enclosed box region. It also sets the
	 * rotation direction of each particle relative to the central direction vector
	 * and the spread value.
	 *
	 * Params:
	 * _projectile - the actor to spawn as projectiles.
	 * _numberOfProjectiles - the number of projectiles to spawn.
	 * _origin - the centre of the bounding box to spawn the projectiles.
	 * _extent - the half size of the bounding box to spawn projectiles.
	 * _centralDirectionUnit - a unit vector desribing the yaw, which is the central direction
	 * where particles will be directed relative from
	 * _spread - the spread of the projectile rotation. As the spread value increases,
	 * the actual spread of projectiles decreases.
	 * _material - the material to apply to the projectiles.
	 * _overlayMaterial - the overlay material to apply to the projectiles.
	 * _scale - the scale of the projectiles.
	 */
	UFUNCTION(BlueprintCallable)
	void SpawnProjectiles(TSubclassOf<AActor> _projectile, int _numberOfProjectiles, FVector _origin,
		 FVector _extent, FVector _centralDirectionUnit, float _spread, UMaterialInterface* _material,
		 UMaterialInterface* _overlayMaterial, FVector _scale = FVector(1, 1, 1));

	/**
	 * Damages the target with poise and earth damage. This occurs if the target actor is considered to be the enemy character of the casting actor.
	 * 
	 * Successive hits on a given enemy by projectiles managed by the same manager instance will result in damage being scaled down.
	 * 
	 * Params:
	 * _caster - the character instigating the damage.
	 * _target - the actor to be damaged.
	 * _earthDamage - the amount of earth damage to apply (pre-scaling from _damageReduceScale).
	 * _poiseDamage - the amount of poise damage to apply.
	 * _damageReduceScale - the scale factor to reduce earth damage by if an actor has already been hit.
	 * _earthDamageClass - responsible for applying earth damage type.
	 * _poiseDamageClass - responsible for applying poise damage type.
	 * Return:
	 * true if damage has been applied, else if damage is unable to be applied, returns false.
	 */
	UFUNCTION(BlueprintCallable)
	bool ApplyDamage(AActor* _target, float _earthDamage, float _poiseDamage, float _damageReduceScale,
		TSubclassOf<UEarthDamageType> _earthDamageClass, TSubclassOf<UPoiseDamageType> _poiseDamageClass);

	/**
	 * Determines whether the two Actors are enemies of each other.
	 *
	 * Params:
	 * _actorOne - the first actor.
	 * _actorTwo - the second actor.
	 * Return:
	 * true if the actors are enemies (and therefore are both derived from AACFCharacter), else false.
	 */
	UFUNCTION(BlueprintCallable)
	bool AreActorsEnemies(AActor* _actorOne, AActor* _actorTwo);
};