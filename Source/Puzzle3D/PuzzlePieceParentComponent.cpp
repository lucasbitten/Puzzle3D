// Fill out your copyright notice in the Description page of Project Settings.

#include "PuzzlePieceParentComponent.h"
#include "PuzzlePawn.h"

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
	
    PuzzleModel = Cast<APuzzleModel>(GetOwner());
    if (!PuzzleModel)
    {
        UE_LOG(LogTemp, Error, TEXT("Puzzle model not found"));
    }

}


void UPuzzlePieceParentComponent::SetIdentifier(FString identifier)
{
    Identifier = identifier;
}

FString UPuzzlePieceParentComponent::GetIdentifier()
{
    return Identifier;
}

void UPuzzlePieceParentComponent::SetLerpCurve(UCurveFloat* Curve)
{
    LerpCurve = Curve;
}

// Called every frame
void UPuzzlePieceParentComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
    Delta = DeltaTime;
    if (IsLerpingToCorrectPositionWithOffset)
    {
        LerpToPositionWithOffsetTimeline.TickTimeline(DeltaTime);
    }

    if (IsLerpingToCorrectPosition)
    {
        LerpToPositionTimeline.TickTimeline(DeltaTime);
    }

    if (IsLerpingCloseToCamera)
    {
        LerpToMoveCloseToCameraTimeline.TickTimeline(DeltaTime);
    }

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

void UPuzzlePieceParentComponent::SetIsLocked(bool locked)
{
    IsLocked = locked;
}

void UPuzzlePieceParentComponent::SetCanLockPieces(bool canLock)
{
    CanLockPieces = canLock;
}

const bool UPuzzlePieceParentComponent::GetIsShell() const
{
    return IsShell;
}

void UPuzzlePieceParentComponent::SetIsShell(bool isShell)
{
    IsShell = isShell;
}

const bool UPuzzlePieceParentComponent::GetIsOnBoard() const
{
    return IsOnBoard;
}

void UPuzzlePieceParentComponent::SetIsOnBoard(bool isOnBoard)
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
    IsLerpingToCorrectPositionWithOffset = true;

    LerpStartPosition =  GetComponentLocation();
    LerpEndPosition = GetParentInitialWorldPositionWithOffset();

    LerpStartRotation = GetComponentRotation();
    LerpEndRotation = GetParentInitialWorldRotator();

    InitializeLerpToPositionWithOffsetTimeline();
}


UPuzzlePiecesComponent* UPuzzlePieceParentComponent::GetPieceMesh() const
{
    return PuzzlePiecesComponent;    
}

void UPuzzlePieceParentComponent::SavePieceInitialRelativeRotation()
{
    if (PuzzlePiecesComponent)
    {
        PieceInitialRelativeRotation = PuzzlePiecesComponent->GetRelativeRotation();
    }
}

void UPuzzlePieceParentComponent::RestorePieceInitialRelativeRotation()
{
    if (PuzzlePiecesComponent)
    {
        PuzzlePiecesComponent->SetRelativeRotation(PieceInitialRelativeRotation);
    }
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



void UPuzzlePieceParentComponent::InitializeLerpCloseToCameraTimeline(float PieceWorldDistanceFromCamera)
{
    if (LerpCurve)
    {
        PieceInWorldDistanceFromCamera = PieceWorldDistanceFromCamera;

        IsLerpingCloseToCamera = true;
        // Bind da fun��o para ser chamada em cada atualiza��o da Timeline
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("HandleLerpCloseToCameraProgress"));

        // Bind para a fun��o de finaliza��o
        FOnTimelineEvent TimelineFinished;
        TimelineFinished.BindUFunction(this, FName("OnLerpCloseToCameraTimelineFinished"));

        // Adiciona as fun��es � Timeline
        LerpToMoveCloseToCameraTimeline.AddInterpFloat(LerpCurve, TimelineProgress);
        LerpToMoveCloseToCameraTimeline.SetTimelineFinishedFunc(TimelineFinished);

        LerpToMoveCloseToCameraTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);


        // Define a Timeline para loop (ou n�o)
        LerpToMoveCloseToCameraTimeline.SetLooping(false);  // N�o � looping

        LerpToMoveCloseToCameraTimeline.PlayFromStart();
    }
}

void UPuzzlePieceParentComponent::StopLerpCloseToCameraTimeline()
{
    LerpToMoveCloseToCameraTimeline.Stop();
}

FVector UPuzzlePieceParentComponent::GetRemovingFinalPosition()
{
    return (GetOffsetDistance() * GetForwardVector() * - 1) + GetComponentLocation();
}

void UPuzzlePieceParentComponent::HandleLerpCloseToCameraProgress(float Value)
{
    FVector NewPosition = FMath::Lerp(GetComponentLocation(), CalculatePositionOutsideModel(), Value);
    SetWorldLocation(NewPosition);

}

void UPuzzlePieceParentComponent::OnLerpCloseToCameraTimelineFinished()
{
    OnLerpToCameraCompletedCallback.Broadcast();
    IsLerpingCloseToCamera = false;
}



void UPuzzlePieceParentComponent::InitializeLerpToPositionWithOffsetTimeline()
{
    if (LerpCurve)
    {
        // Bind da fun��o para ser chamada em cada atualiza��o da Timeline
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("HandleLerpWithOffsetProgress"));

        // Bind para a fun��o de finaliza��o
        FOnTimelineEvent TimelineFinished;
        TimelineFinished.BindUFunction(this, FName("OnLerpToPositionWithOffsetTimelineFinished"));

        // Adiciona as fun��es � Timeline
        LerpToPositionWithOffsetTimeline.AddInterpFloat(LerpCurve, TimelineProgress);
        LerpToPositionWithOffsetTimeline.SetTimelineFinishedFunc(TimelineFinished);

        LerpToPositionWithOffsetTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);

        // Define a Timeline para loop (ou n�o)
        LerpToPositionWithOffsetTimeline.SetLooping(false);  // N�o � looping

        LerpToPositionWithOffsetTimeline.PlayFromStart();
    }
}

void UPuzzlePieceParentComponent::HandleLerpWithOffsetProgress(float Value)
{
    // Interpolando a posi��o
    FVector NewPosition = FMath::Lerp(LerpStartPosition, LerpEndPosition, Value);
    SetWorldLocation(NewPosition);

    // Interpolando a rota��o usando RInterpTo com base no valor
    FRotator NewRotation = FMath::RInterpTo(GetComponentRotation(), LerpEndRotation, Delta, LerpRotationSpeed);
    SetWorldRotation(NewRotation);

    GetPieceMesh()->SetRelativeRotation(PieceInitialRelativeRotation);
}

void UPuzzlePieceParentComponent::OnLerpToPositionWithOffsetTimelineFinished()
{
    IsLerpingToCorrectPositionWithOffset = false;
    InitializeLerpToCorrectPositionTimeline();

}



void UPuzzlePieceParentComponent::InitializeLerpToCorrectPositionTimeline()
{
    if (LerpCurve)
    {
        PuzzleModel->PlayFittingSound();

        IsLerpingToCorrectPosition = true;

        LerpStartPosition = GetParentInitialWorldPositionWithOffset();
        LerpEndPosition = GetParentInitialWorldPosition();

        LerpStartRotation = GetComponentRotation();
        LerpEndRotation = GetParentInitialWorldRotator();


        // Bind da fun��o para ser chamada em cada atualiza��o da Timeline
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("HandleLerpToCorrectPositionProgress"));

        // Bind para a fun��o de finaliza��o
        FOnTimelineEvent TimelineFinished;
        TimelineFinished.BindUFunction(this, FName("OnLerpToCorrectPositionTimelineFinished"));

        // Adiciona as fun��es � Timeline
        LerpToPositionTimeline.AddInterpFloat(LerpCurve, TimelineProgress);
        LerpToPositionTimeline.SetTimelineFinishedFunc(TimelineFinished);

        LerpToPositionTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);


        // Define a Timeline para loop (ou n�o)
        LerpToPositionTimeline.SetLooping(false);  // N�o � looping

        LerpToPositionTimeline.PlayFromStart();
    }
}

void UPuzzlePieceParentComponent::HandleLerpToCorrectPositionProgress(float Value)
{

    // Interpolando a posi��o
    FVector NewPosition = FMath::Lerp(LerpStartPosition, LerpEndPosition, Value);
    SetWorldLocation(NewPosition);

    //// Interpolando a rota��o usando RInterpTo com base no valor
    //FRotator NewRotation = FMath::RInterpTo(GetComponentRotation(), LerpEndRotation, Delta, LerpRotationSpeed);
    //SetWorldRotation(NewRotation);

    //GetPieceMesh()->SetRelativeRotation(PieceInitialRelativeRotation);

}

void UPuzzlePieceParentComponent::OnLerpToCorrectPositionTimelineFinished()
{
    IsLerpingToCorrectPosition = false;

    SetWorldLocation(InitialParentWorldPosition);
    SetWorldRotation(InitialParentWorldRotator);
    GetPieceMesh()->SetRelativeRotation(PieceInitialRelativeRotation);

    SetIsLocked(true);
    OnLerpToCorrectPositionCompletedCallback.Broadcast();
    if (PuzzleModel)
    {
        PuzzleModel->OnPiecePlaced(this);
    }
}

void UPuzzlePieceParentComponent::OnReleasedIncorrectPiece()
{
    if (PuzzleModel)
    {
        PuzzleModel->OnPieceDropped(this);
    }
}

void UPuzzlePieceParentComponent::SetBoardProperties(USceneComponent* parent, FVector position, FRotator rotation, FVector scale)
{
    boardParent = parent;
    boardPosition = position;
    boardRotation = rotation;
    boardScale = scale;
}

void UPuzzlePieceParentComponent::ResetToBoard()
{
    AttachToComponent(boardParent);
    SetWorldLocation(boardPosition);
    SetRelativeRotation(boardRotation);
    SetWorldScale3D(boardScale);
}


FVector UPuzzlePieceParentComponent::CalculatePositionOutsideModel()
{
    APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
    if (PlayerController)
    {
        FVector WorldLocation, WorldDirection;
        if (PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
        {
            return WorldLocation + (WorldDirection * PieceInWorldDistanceFromCamera);
        }
    }
    return FVector::Zero();
}
