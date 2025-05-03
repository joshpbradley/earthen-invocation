// Copyright (C) I N F I N I T Y 27 LTD. All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "../SamsaraBaseSpell_cpp.h"

#include "EarthenInvocationMantra.generated.h"

/**
 * Earthen Invocation mantra.
 */
UCLASS()
class SAMSARA_API UEarthenInvocationMantra : public USamsaraBaseSpell_cpp
{
	GENERATED_BODY()

protected:
	/*
	 * The unit vector representing the yaw that the boulder/projectiles in the spell will launch in.
	 */
	UPROPERTY(BlueprintReadOnly)
	FVector spellDirection;

	/*
	 * The XY world location that the boulder will spawn from.
	 */
	UPROPERTY(BlueprintReadOnly)
	FVector2D boulderSpawnLocation;

private:
	/*
	 * The caster of the spell.
	 * 
	 * This is used to retrieve the World reference instead of characterOwner in the parent class, which is unset prior to the first spell activation.
	 */
	ACharacter* caster = nullptr;

protected:
	/**
	 * Gets the current World reference, which is required for performing line trace operations.
     *
     * The override is necessary because the base implementation of GetWorld() depends on characterOwner to retrieve the World reference,
     * and characterOwner is unset prior to the first spell activation.
	 */
	UWorld* GetWorld() const override;

	/**
	 * Calculates and returns the unit vector of the spell's direction. This will be directed towards a targeted enemy if
	 * the caster is targeting, else it will be in the forward direction of the caster.
	 * 
	 * Return:
	 * the unit vector direction of the spell.
	 */
	FVector CalculateSpellDirection();

	/**
	 * Determines whether there is flat ground directly in front of the caster, so that the boulder has a suitable surface to spawn from.
	 * 
	 * Params:
	 * _caster - the caster of the spell.
	 * _casterAndBoulderHorizontalDistance - the horizontal distance between the caster and the boulder required to give the illusion of the boulder being struck.
	 * This will approximately be the boulder radius + the forward distance travelled by the caster during the cast animation.
	 * _angleTolerance - the maximum angle relative to horizontal plane that the spell can be cast at.
	 * _maximumHeightDiscrepancyBetweenCasterAndSurface - the maximum height difference tolerated between the location where the caster is stood and the location for spawning the boulder.
	 * _lineTraceHeight - the height of the line trace that detects an actor representing the floor.
	 * 
	 * Return:
	 * true if flat ground has been detected in front of the caster, else false.
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool CanExecuteAction_Custom(ACharacter* _caster, float _casterAndBoulderHorizontalDistance,
		float _angleTolerance, float _maximumHeightDiscrepancyBetweenCasterAndSurface, float _lineTraceHeight);

	/**
	 * Sets the spellDirection variable for the duration of the spell.
	 */
	UFUNCTION(BlueprintCallable)
	void SetSpellDirection();
};