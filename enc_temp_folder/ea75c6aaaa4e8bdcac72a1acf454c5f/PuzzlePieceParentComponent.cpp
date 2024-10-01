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
	// ...

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
    IsLerpingToCorrectPositionWithOffset = true;

    LerpStartPosition =  GetComponentLocation();
    LerpEndPosition = GetParentInitialWorldPositionWithOffset();

    LerpStartRotation = GetComponentRotation();
    LerpEndRotation = GetParentInitialWorldRotator();

    UE_LOG(LogTemp, Warning, TEXT("LerpEnd Rotation: %s"), *LerpEndRotation.ToString());
    UE_LOG(LogTemp, Warning, TEXT("PieceInitialRelativeRotation: %s"), *PieceInitialRelativeRotation.ToString());

    
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

void UPuzzlePieceParentComponent::InitializeLerpToPositionWithOffsetTimeline()
{
    if (LerpCurve)
    {
        // Bind da função para ser chamada em cada atualização da Timeline
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("HandleLerpWithOffsetProgress"));

        // Bind para a função de finalização
        FOnTimelineEvent TimelineFinished;
        TimelineFinished.BindUFunction(this, FName("OnLerpToPositionWithOffsetTimelineFinished"));

        // Adiciona as funções à Timeline
        LerpToPositionWithOffsetTimeline.AddInterpFloat(LerpCurve, TimelineProgress);
        LerpToPositionWithOffsetTimeline.SetTimelineFinishedFunc(TimelineFinished);

        LerpToPositionWithOffsetTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);


        // Define a Timeline para loop (ou não)
        LerpToPositionWithOffsetTimeline.SetLooping(false);  // Não é looping

        LerpToPositionWithOffsetTimeline.PlayFromStart();
    }
}


void UPuzzlePieceParentComponent::InitializeLerpCloseToCameraTimeline(float PieceWorldDistanceFromCamera)
{
    if (LerpCurve)
    {
        PieceInWorldDistanceFromCamera = PieceWorldDistanceFromCamera;
        // Bind da função para ser chamada em cada atualização da Timeline
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("HandleLerpCloseToCameraProgress"));

        // Bind para a função de finalização
        FOnTimelineEvent TimelineFinished;
        TimelineFinished.BindUFunction(this, FName("OnLerpCloseToCameraTimelineFinished"));

        // Adiciona as funções à Timeline
        LerpToPositionWithOffsetTimeline.AddInterpFloat(LerpCurve, TimelineProgress);
        LerpToPositionWithOffsetTimeline.SetTimelineFinishedFunc(TimelineFinished);

        LerpToPositionWithOffsetTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);


        // Define a Timeline para loop (ou não)
        LerpToPositionWithOffsetTimeline.SetLooping(false);  // Não é looping

        LerpToPositionWithOffsetTimeline.PlayFromStart();
    }
}



void UPuzzlePieceParentComponent::HandleLerpCloseToCameraProgress(float Value)
{
    FVector NewPosition = FMath::Lerp(GetComponentLocation(), CalculatePositionOutsideModel(), Value);
    SetWorldLocation(NewPosition);

}

void UPuzzlePieceParentComponent::OnLerpCloseToCameraTimelineFinished()
{
    OnLerpCompletedCallback.Broadcast();
}

void UPuzzlePieceParentComponent::HandleLerpWithOffsetProgress(float Value)
{
    // Interpolando a posição
    FVector NewPosition = FMath::Lerp(LerpStartPosition, LerpEndPosition, Value);
    SetWorldLocation(NewPosition);

    // Interpolando a rotação usando RInterpTo com base no valor
    FRotator NewRotation = FMath::RInterpTo(GetComponentRotation(), LerpEndRotation, Delta, LerpRotationSpeed);
    SetWorldRotation(NewRotation);

    GetPieceMesh()->SetRelativeRotation(PieceInitialRelativeRotation);
}

void UPuzzlePieceParentComponent::HandleLerpProgress(float Value)
{

    // Interpolando a posição
    FVector NewPosition = FMath::Lerp(LerpStartPosition, LerpEndPosition, Value);
    SetWorldLocation(NewPosition);

    //// Interpolando a rotação usando RInterpTo com base no valor
    //FRotator NewRotation = FMath::RInterpTo(GetComponentRotation(), LerpEndRotation, Delta, LerpRotationSpeed);
    //SetWorldRotation(NewRotation);

    //GetPieceMesh()->SetRelativeRotation(PieceInitialRelativeRotation);

}

void UPuzzlePieceParentComponent::OnLerpToPositionWithOffsetTimelineFinished()
{
    IsLerpingToCorrectPositionWithOffset = false;
    IsLerpingToCorrectPosition = true;

    // Lógica para quando a Timeline terminar
    UE_LOG(LogTemp, Warning, TEXT("Primeiro Lerp concluído!"));

    if (LerpCurve)
    {
        LerpStartPosition = GetParentInitialWorldPositionWithOffset();
        LerpEndPosition = GetParentInitialWorldPosition();

        LerpStartRotation = GetComponentRotation();
        LerpEndRotation = GetParentInitialWorldRotator();


        // Bind da função para ser chamada em cada atualização da Timeline
        FOnTimelineFloat TimelineProgress;
        TimelineProgress.BindUFunction(this, FName("HandleLerpProgress"));

        // Bind para a função de finalização
        FOnTimelineEvent TimelineFinished;
        TimelineFinished.BindUFunction(this, FName("OnLerpToPositionTimelineFinished"));

        // Adiciona as funções à Timeline
        LerpToPositionTimeline.AddInterpFloat(LerpCurve, TimelineProgress);
        LerpToPositionTimeline.SetTimelineFinishedFunc(TimelineFinished);

        LerpToPositionTimeline.SetTimelineLengthMode(ETimelineLengthMode::TL_LastKeyFrame);


        // Define a Timeline para loop (ou não)
        LerpToPositionTimeline.SetLooping(false);  // Não é looping

        LerpToPositionTimeline.PlayFromStart();
    }

}

void UPuzzlePieceParentComponent::OnLerpToPositionTimelineFinished()
{
    // Lógica para quando a Timeline terminar
    UE_LOG(LogTemp, Warning, TEXT("Lerp concluído!"));
    IsLerpingToCorrectPosition = false;

    SetWorldLocation(InitialParentWorldPosition);
    SetWorldRotation(InitialParentWorldRotator);
    GetPieceMesh()->SetRelativeRotation(PieceInitialRelativeRotation);


    // todo?
    // GetPieceMesh()->SetMaterial(1, OriginalMaterial);
    // DeselectPieceComponent()
    SetIsLocked(true);
    OnLerpCompletedCallback.Broadcast();

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
