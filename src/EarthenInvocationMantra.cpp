// Copyright (C) I N F I N I T Y 27 LTD. All rights reserved. 

#include "EarthenInvocationMantra.h"

#include <Kismet/KismetMathLibrary.h>

/**
 * Gets the current World reference, which is required for performing line trace operations.
 *
 * The override is necessary because the base implementation of GetWorld() depends on characterOwner to retrieve the World reference,
 * and characterOwner is unset prior to the first spell activation.
 */
UWorld* UEarthenInvocationMantra::GetWorld() const
{
	return caster ? caster->GetWorld() : nullptr;
}

/**
 * Determines whether there is flat ground directly in front of the caster, so that the boulder has a suitable surface to spawn from.
 */
bool UEarthenInvocationMantra::CanExecuteAction_Custom(ACharacter* _caster, float _casterAndBoulderHorizontalDistance,
	float _angleTolerance, float _maximumHeightDiscrepancyBetweenCasterAndSurface, float _lineTraceHeight)
{
	if(!caster)
	{
		caster = _caster;
	}

	// Determines whether the player has a conch to consume and is not experiencing ragdoll physics.
	if (!USamsaraBaseSpell_cpp::CanExecuteAction_Implementation(caster))
	{
		return false;
	}
	
	// Gets the distance between the caster and boulder using the arm length of the caster.
	auto skeletalMeshForSkeleton = Cast<USkeletalMeshComponent>(caster->GetComponentByClass(USkeletalMeshComponent::StaticClass()));
	float upperArmLength = UKismetMathLibrary::Vector_Distance(skeletalMeshForSkeleton->GetSocketLocation("upperarm_r"), skeletalMeshForSkeleton->GetSocketLocation("lowerarm_r"));
	float lowerArmLength = UKismetMathLibrary::Vector_Distance(skeletalMeshForSkeleton->GetSocketLocation("lowerarm_r"), skeletalMeshForSkeleton->GetSocketLocation("RightHandSocket"));
	float torsoLength = UKismetMathLibrary::Vector_Distance(skeletalMeshForSkeleton->GetSocketLocation("spine_02"), skeletalMeshForSkeleton->GetSocketLocation("spine_05"));
	float boulderDistanceFromCaster = _casterAndBoulderHorizontalDistance + upperArmLength + lowerArmLength + torsoLength;

	// The displacement from the caster to the boulder.
	FVector2D boulderDisplacementFromCaster = FVector2D(CalculateSpellDirection()) * boulderDistanceFromCaster;

	// Gets the bounds of the caster's mesh.
	auto skeletalMeshForBounds = Cast<USkeletalMeshComponent>(caster->GetComponentsByTag(USkeletalMeshComponent::StaticClass(), FName("Body"))[0]);
	FBoxSphereBounds bounds = skeletalMeshForBounds->Bounds;

	// The location where the caster contacts the floor.
	FVector casterStandingPosition = FVector(bounds.Origin.X, bounds.Origin.Y, bounds.Origin.Z - bounds.BoxExtent.Z);

	// The central location of the trace - where the boulder will spawn from.
	boulderSpawnLocation = FVector2D(casterStandingPosition) + boulderDisplacementFromCaster;

	// Stores the hit result of the line trace.
	FHitResult hit;

	// The beginning and end locations of the line trace.
	FVector start(boulderSpawnLocation.X, boulderSpawnLocation.Y, casterStandingPosition.Z + _lineTraceHeight / 2);
	FVector end(boulderSpawnLocation.X, boulderSpawnLocation.Y, casterStandingPosition.Z - _lineTraceHeight / 2);

	// Query for WorldStatic objects.
	FCollisionObjectQueryParams objectTypes;
	objectTypes.AddObjectTypesToQuery(ECC_WorldStatic);

	// Ensures the caster does not interfere with collision responses.
	FCollisionQueryParams params;
	params.AddIgnoredActor(caster);

	GetWorld()->LineTraceSingleByObjectType(hit, start, end, objectTypes, params);

	// No surface was found, or the difference in height between the caster and the detected surface is too great to cast.
	if (!hit.bBlockingHit || abs(hit.Location.Z - casterStandingPosition.Z) > _maximumHeightDiscrepancyBetweenCasterAndSurface)
	{
		return false;
	}
	// Determine whether the surface is flat enough to cast.
	else
	{
		return (90 - UKismetMathLibrary::MakeRotFromX(hit.ImpactNormal).Pitch) <= _angleTolerance;
	}
}

/**
 * Calculates and returns the unit vector of the spell's direction. This will be directed towards a targeted enemy if
 * the caster is targeting, else it will be in the forward direction of the caster.
 */
FVector UEarthenInvocationMantra::CalculateSpellDirection()
{
	// Return an enemy reference if the caster is locked on.
	AActor* target = GetSpellTarget(caster, 0);

	// Target has been acquired, so set the direction towards them.
	if(target)
	{
		FVector direction = target->GetActorLocation() - caster->GetActorLocation();
		direction.Z = 0;
		direction.Normalize();

		return direction;
	}
	// No target acquired so return the forward vector of the caster.
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