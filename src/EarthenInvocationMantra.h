// Copyright (C) I N F I N I T Y 27 LTD. All rights reserved. 

#pragma once

#include "CoreMinimal.h"
#include "../SamsaraBaseSpell_cpp.h"

#include "EarthenInvocationMantra.generated.h"

/**
 * The Earthen Invocation mantra.
 */
UCLASS()
class SAMSARA_API UEarthenInvocationMantra : public USamsaraBaseSpell_cpp
{
	GENERATED_BODY()

protected:
	/*
	 * The unit vector representing the direction that the boulder / projectiles in the spell will launch in.
	 * This direction should have a Z-component equal to 0.
	 */
	UPROPERTY(BlueprintReadOnly)
	FVector spellDirection;

	/*
	 * The horizontal distance in units between the caster and where the location that the boulder should spawn.
	 */
	UPROPERTY(BlueprintReadOnly)
	float boulderDistanceFromCaster = -1;

private:
	/*
	 * The caster of the spell. This actor should match the CharacterOwner property in the parent class.
	 * 
	 * The reason for having a private copy (caster) of CharacterOwner is because CharacterOwner is only set upon the first activation
	 * of the spell, and the actor is required to determine whether the spell can be activated - i.e., prior to the first spell activation.
	 */
	ACharacter* caster = nullptr;

protected:
	/**
	 * Gets the world context associated with the casting of this spell. The override is necessary so that
	 * the caster variable is used to retrieve the world context instead of CharacterOwner, which may not have been set.
	 */
	UWorld* GetWorld() const override;

	/**
	 * Calculates and returns the unit vector of the spell's direction. This will be directly towards a targeted enemy if
	 * the player is targeting, else it will be in the forward direction of the caster.
	 * 
	 * Return:
	 * the unit vector direction of the spell.
	 */
	FVector CalculateSpellDirection();

	/**
	 * The custom implementation that is executed on each CanExecuteAction call, invoked from blueprints.
	 * This implementation includes a call to the parent function of CanExecuteAction, present in SamsaraBaseSpell_cpp.
	 * Determines whether solid ground is directly in front of the caster so that the boulder has a surface to spawn from.
	 * 
	 * Params:
	 * _caster - the caster of the spell.
	 * _castAnimationDistance - the horizontal distance that the caster travels when casting the spell.
	 * _lineTraceHeight - the vertical distance that should be inspected for solid ground in a line trace.
	 * Return:
	 * true if ground has been detected, and so the spell can be casted, else false.
	 */
	UFUNCTION(BlueprintCallable)
	virtual bool CanExecuteAction_Custom(ACharacter* _caster, float _castAnimationDistance, float _lineTraceHeight);

	/**
	 * Sets the spellDirection variable for the duration of the spell.
	 */
	UFUNCTION(BlueprintCallable)
	void SetSpellDirection();
};