// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePiecesComponent.h"
#include "Kismet/KismetMathLibrary.h"


void UPuzzlePiecesComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UPuzzlePiecesComponent::SetParentInitialWorldPosition(FVector initialParentWorldPos)
{
    InitialParentWorldPosition = initialParentWorldPos;
}

const FVector UPuzzlePiecesComponent::GetParentInitialWorldPosition() const
{
	return InitialParentWorldPosition;
}

void UPuzzlePiecesComponent::SetParentInitialWorldPositionWithOffset(FVector initialPos)
{
    InitialParentWorldPositionWithOffset = initialPos;
}

const FVector UPuzzlePiecesComponent::GetParentInitialWorldPositionWithOffset() const
{
    return InitialParentWorldPositionWithOffset;
}

void UPuzzlePiecesComponent::SetParentInitialWorldRotator(FRotator initialRotation)
{
    InitialParentWorldRotator = initialRotation;
}

const FRotator UPuzzlePiecesComponent::GetParentInitialWorldRotator() const
{
    return InitialParentWorldRotator;
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

void UPuzzlePiecesComponent::SetOffsetDistance(float offset)
{
    OffsetDistance = offset;
}

float UPuzzlePiecesComponent::GetOffsetDistance()
{
    return OffsetDistance;
}
