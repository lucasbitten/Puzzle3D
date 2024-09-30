#include "PuzzlePieceParentComponent.h"
// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePieceParentComponent.h"

// Sets default values for this component's properties
UPuzzlePieceParentComponent::UPuzzlePieceParentComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UPuzzlePieceParentComponent::BeginPlay()
{
	Super::BeginPlay();
    SetPieceMesh();
	// ...
	
}


// Called every frame
void UPuzzlePieceParentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UPuzzlePieceParentComponent::SetParentInitialWorldPosition(FVector initialParentWorldPos)
{
    InitialParentWorldPosition = initialParentWorldPos;
}

const FVector UPuzzlePieceParentComponent::GetParentInitialWorldPosition() const
{
    return InitialParentWorldPosition;
}

void UPuzzlePieceParentComponent::SetParentInitialWorldPositionWithOffset(FVector initialPos)
{
    InitialParentWorldPositionWithOffset = initialPos;
}

const FVector UPuzzlePieceParentComponent::GetParentInitialWorldPositionWithOffset() const
{
    return InitialParentWorldPositionWithOffset;
}

void UPuzzlePieceParentComponent::SetParentInitialWorldRotator(FRotator initialRotation)
{
    InitialParentWorldRotator = initialRotation;
}

const FRotator UPuzzlePieceParentComponent::GetParentInitialWorldRotator() const
{
    return InitialParentWorldRotator;
}


const bool UPuzzlePieceParentComponent::GetIsLocked() const
{
    if (CanLockPieces)
    {
        return IsLocked;
    }

    return false;

}

const void UPuzzlePieceParentComponent::SetIsLocked(bool locked)
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

const void UPuzzlePieceParentComponent::SetCanLockPieces(bool canLock)
{
    CanLockPieces = canLock;
}

const bool UPuzzlePieceParentComponent::GetIsShell() const
{
    return IsShell;
}

const void UPuzzlePieceParentComponent::SetIsShell(bool isShell)
{
    IsShell = isShell;
}

const bool UPuzzlePieceParentComponent::GetIsOnBoard() const
{
    return IsOnBoard;
}

const void UPuzzlePieceParentComponent::SetIsOnBoard(bool isOnBoard)
{
    IsOnBoard = isOnBoard;
}

void UPuzzlePieceParentComponent::SetOffsetDistance(float offset)
{
    OffsetDistance = offset;
}

float UPuzzlePieceParentComponent::GetOffsetDistance()
{
    return OffsetDistance;
}

void UPuzzlePieceParentComponent::StartLerpingToCorrectPositionWithOffset()
{
    LerpStartPosition =  GetComponentLocation();
    LerpEndPosition = GetParentInitialWorldPositionWithOffset();

    LerpStartRotation = GetComponentRotation();
    LerpEndRotation = GetParentInitialWorldRotator();

    IsLerping = true;

}

UPuzzlePiecesComponent* UPuzzlePieceParentComponent::GetPieceMesh() const
{
    return PuzzlePiecesComponent;    
}

void UPuzzlePieceParentComponent::SetPieceMesh()
{
    if (GetNumChildrenComponents() > 0)
    {
        if (UPuzzlePiecesComponent* PuzzlePiece = Cast<UPuzzlePiecesComponent>(GetChildComponent(0)))
        {
            PuzzlePiecesComponent = PuzzlePiece;
        }
    }
}
