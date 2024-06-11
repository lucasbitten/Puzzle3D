// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzlePiecesComponent.h"
#include "Kismet/KismetMathLibrary.h"

void UPuzzlePiecesComponent::SetInitialRelativePosition(FVector initialRelativePos)
{
    InitialRelativePosition = initialRelativePos;
}

const FVector UPuzzlePiecesComponent::GetInitialRelativePosition() const
{
	return InitialRelativePosition;
}

void UPuzzlePiecesComponent::BeginPlay()
{
    Super::BeginPlay();
    SetInitialRelativePosition(GetRelativeLocation());
}


void UPuzzlePiecesComponent::SetInitialNormal(FVector EndLocation)
{

    FVector StartLocation = GetComponentLocation();

    FHitResult HitResult;
    FCollisionQueryParams CollisionParams;
    CollisionParams.AddIgnoredComponent(this);
    CollisionParams.bTraceComplex = true;

    ECollisionChannel TraceChannel = ECC_GameTraceChannel2;

    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        UE_LOG(LogTemp, Warning, TEXT("No Editor World found"));
        return;
    }


    bool bHit = World->LineTraceSingleByChannel(
        HitResult,
        StartLocation,
        EndLocation,
        ECollisionChannel::ECC_WorldDynamic,
        CollisionParams
    );

    if (bHit)
    {
        UE_LOG(LogTemp, Log, TEXT("Hit: %s"), *HitResult.GetActor()->GetName());
        InitialNormal = HitResult.ImpactNormal;
    }
    else
    {
        UE_LOG(LogTemp, Log, TEXT("No Hit"));
    }
}

const FVector UPuzzlePiecesComponent::GetInitialNormal() const
{
	return InitialNormal;
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
    return IsLocked;
}

const void UPuzzlePiecesComponent::SetIsLocked(bool locked)
{
    IsLocked = locked;
}

const bool UPuzzlePiecesComponent::GetIsShell() const
{
    return IsShell;
}

const void UPuzzlePiecesComponent::SetIsShell(bool isShell)
{
    IsShell = isShell;
}

const void UPuzzlePiecesComponent::AlignToSurfaceNormal(FVector currentNormal)
{
    FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(GetComponentLocation(), currentNormal);

    //FRotator RotacaoInicial = UKismetMathLibrary::FindLookAtRotation(InitialPosition, InitialNormal);
    //FRotator RotacaoFinal = UKismetMathLibrary::FindLookAtRotation(GetRelativeLocation(), currentNormal);
    //FRotator RotacaoIncremental = RotacaoFinal - RotacaoInicial;
    //FRotator NovaRotacao = GetRelativeRotation() + RotacaoIncremental;
    SetRelativeRotation(LookAtRotation);

}
