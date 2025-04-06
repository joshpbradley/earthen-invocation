// Copyright (C) I N F I N I T Y 27 LTD. All rights reserved. 

#include "EarthenInvocationMantra.h"

#include "Engine/SkinnedAsset.h"

/**
 * Gets the world context associated with the casting of this spell. The override is necessary so that
 * the caster variable is used to retrieve the world context instead of CharacterOwner, which may not have been set.
 */
UWorld* UEarthenInvocationMantra::GetWorld() const
{
	return caster ? caster->GetWorld() : nullptr;
}

/**
 * The custom implementation that is executed on each CanExecuteAction call, invoked from blueprints.
 * This implementation includes a call to the parent function of CanExecuteAction, present in SamsaraBaseSpell_cpp.
 * Determines whether solid ground is directly in front of the caster so that the boulder has a surface to spawn from.
 */
bool UEarthenInvocationMantra::CanExecuteAction_Custom(ACharacter* _caster, float _castAnimationDistance, float _lineTraceHeight)
{
	if(!caster)
	{
		caster = _caster;
	}

	// Sets the boulder distance from the caster based on the caster's armspan and a given distance that the caster travels through in the animation.
	if(boulderDistanceFromCaster < 0)
	{
		USkinnedAsset* mesh = Cast<USkeletalMeshComponent>(caster->GetComponentByClass(USkeletalMeshComponent::StaticClass()))->GetSkinnedAsset();
		boulderDistanceFromCaster = _castAnimationDistance + mesh->GetBounds().BoxExtent.X;
	}

	// Determines whether the player has a conch to consume and is not experiencing ragdoll physics.
	if(!USamsaraBaseSpell_cpp::CanExecuteAction_Implementation(caster))
	{
		return false;
	}

	// The boulder's displacement relative to the caster.
	FVector boulderDisplacementFromCaster = CalculateSpellDirection() * boulderDistanceFromCaster;
	// The location where the caster contacts the floor.
	FVector casterStandingPosition;

	// Gets the bounds of the caster's mesh.
	auto mesh = Cast<USkeletalMeshComponent>(caster->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), FName("Body"))[0]);

	FBoxSphereBounds bounds = mesh->Bounds;

	casterStandingPosition = FVector(bounds.Origin.X, bounds.Origin.Y, bounds.Origin.Z - bounds.BoxExtent.Z);

	/*
	 * The central location of the trace. A floor surface hit should occur at this location, or slightly above/below
	 * (within a given height range, _lineTraceHeight), for the spell activation to be valid.
	 */
	auto traceLocation = casterStandingPosition + boulderDisplacementFromCaster;

	FVector start(traceLocation.X, traceLocation.Y, traceLocation.Z + (_lineTraceHeight / 2));
	FVector end(traceLocation.X, traceLocation.Y, traceLocation.Z - (_lineTraceHeight / 2));

	// Query for WorldStatic objects.
	FCollisionObjectQueryParams objectTypes;
	objectTypes.AddObjectTypesToQuery(ECC_WorldStatic);

	// Ensures the caster does not interfere with collision responses.
	FCollisionQueryParams params;
	params.AddIgnoredActor(caster);

    return GetWorld()->LineTraceTestByObjectType(start, end, objectTypes, params);
}

/**
 * Calculates and returns the unit vector of the spell's direction. This will be directly towards a targeted enemy if
 * the player is targeting, else it will be in the forward direction of the caster.
 */
FVector UEarthenInvocationMantra::CalculateSpellDirection()
{
	AActor* target = GetSpellTarget(caster, 0);

	// Target has been acquired, so set the direction towards them and parallel to the floor.
	if(target)
	{
		FVector direction = target->GetActorLocation() - caster->GetActorLocation();
		direction.Z = 0;
		direction.Normalize();

		return direction;
	}
	// No target acquired so set the spell direction to the forward vector of the caster.
	else
	{
		return caster->GetActorForwardVector();
	}
}

/**
 * Sets the spellDirection variable for the duration of the spell.
 */
void UEarthenInvocationMantra::SetSpellDirection()
{
	spellDirection = CalculateSpellDirection();
}