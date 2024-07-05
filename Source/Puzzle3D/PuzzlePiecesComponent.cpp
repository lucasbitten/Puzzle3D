// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePiecesComponent.h"
#include "Kismet/KismetMathLibrary.h"


void UPuzzlePiecesComponent::BeginPlay()
{
    Super::BeginPlay();

    CalculateInitialRotation();
}

void UPuzzlePiecesComponent::SetParentInitialRelativePosition(FVector initialParentRelativePos)
{
    InitialParentRelativePosition = initialParentRelativePos;
}

const FVector UPuzzlePiecesComponent::GetParentInitialRelativePosition() const
{
	return InitialParentRelativePosition;
}

void UPuzzlePiecesComponent::SetParentInitialRelativeRotator(FRotator initialRotation)
{
    InitialParentRelativeRotator = initialRotation;
}

const FRotator UPuzzlePiecesComponent::GetParentInitialRelativeRotator() const
{
    return InitialParentRelativeRotator;
}

void UPuzzlePiecesComponent::CalculateRotationOffset()
{
    if (USceneComponent* ParentComponent = GetAttachParent())
    {
        FRotator ParentRotation = ParentComponent->GetComponentRotation();
        FRotator ChildRotation = GetRelativeRotation();

        // Calculate the rotation offset
        RotationOffset = ChildRotation - ParentRotation;
    }
}

FRotator UPuzzlePiecesComponent::GetRotationOffset() const
{
    return RotationOffset;
}


void UPuzzlePiecesComponent::SetShellRelativePosition(FVector position)
{
    ShellRelativePosition = position;
}

const FVector UPuzzlePiecesComponent::GetShellRelativePosition() const
{
    return ShellRelativePosition;
}

const bool UPuzzlePiecesComponent::GetIsLocked() const
{
    if (CanLockPieces)
    {
        return IsLocked;
    }
    
    return false;

}

const void UPuzzlePiecesComponent::SetIsLocked(bool locked)
{
    if (CanLockPieces)
    {
        IsLocked = locked;
    }
    else
    {
        IsLocked = false;
    }
}

const void UPuzzlePiecesComponent::SetCanLockPieces(bool canLock)
{
    CanLockPieces = canLock;
}

const bool UPuzzlePiecesComponent::GetIsShell() const
{
    return IsShell;
}

const void UPuzzlePiecesComponent::SetIsShell(bool isShell)
{
    IsShell = isShell;
}

// Function to calculate and store initial rotation
void UPuzzlePiecesComponent::CalculateInitialRotation()
{
    USceneComponent* MyParentComponent = Cast<USceneComponent>(GetAttachParent());
    if (MyParentComponent == nullptr)
    {
        return;
    }
    InitialRotation = MyParentComponent->GetComponentRotation();

    SetParentInitialRelativePosition(MyParentComponent->GetRelativeLocation());
    SetParentInitialRelativeRotator(MyParentComponent->GetRelativeRotation());
    CalculateRotationOffset();

    InitialQuat = FRotationMatrix::MakeFromX(MyParentComponent->GetForwardVector()).ToQuat();
}

void UPuzzlePiecesComponent::MoveParentToSurface(USceneComponent* ParentComponent, FVector ImpactPoint, FVector ImpactNormal, float Offset)
{
    if (ParentComponent)
    {
        FVector OffsetPosition = ImpactPoint + (ImpactNormal * Offset);

        // Set the location of the parent to the impact point
        ParentComponent->SetWorldLocation(OffsetPosition);

        // Calculate the rotation such that the Z-axis of the parent points towards the impact normal
        FRotator LookAtRotation = FRotationMatrix::MakeFromX(ImpactNormal).Rotator();

        // Combine the look-at rotation with the initial rotation to maintain the correct orientation
        FRotator NewRotation = LookAtRotation + InitialRotation;
        ParentComponent->SetWorldRotation(NewRotation);
    }
}