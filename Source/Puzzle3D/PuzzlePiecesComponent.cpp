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

float UPuzzlePiecesComponent::CalculateBlendFactor(float Value, float Threshold)
{
    return FMath::Clamp(FMath::Abs(Value) / Threshold, 0.0f, 1.0f);
}



void UPuzzlePiecesComponent::MoveParentToSurface(USceneComponent* ParentComponent, FVector ImpactPoint, FVector ImpactNormal, float Offset)
{
    if (ParentComponent)
    {
        FVector OffsetPosition = ImpactPoint + (ImpactNormal * Offset);

        // Set the location of the parent to the offset position
        ParentComponent->SetWorldLocation(OffsetPosition);

        // Determine the appropriate rotation based on the initial rotation and position
        FVector ForwardVector = -ImpactNormal;

        // Calculate rotations for each axis
        FQuat LookAtQuatX = FRotationMatrix::MakeFromX(ForwardVector).ToQuat();
        FQuat LookAtQuatY = FRotationMatrix::MakeFromY(ForwardVector).ToQuat();
        FQuat LookAtQuatZ = FRotationMatrix::MakeFromZ(ForwardVector).ToQuat();

        // Determine the blend factors based on the position relative to the center
        FVector Center = FVector::ZeroVector;
        FVector PositionRelativeToCenter = OffsetPosition - Center;

        float BlendFactorX = CalculateBlendFactor(PositionRelativeToCenter.X, 1.0f); // Adjust threshold as needed
        float BlendFactorY = CalculateBlendFactor(PositionRelativeToCenter.Y, 1.0f); // Adjust threshold as needed
        float BlendFactorZ = CalculateBlendFactor(PositionRelativeToCenter.Z, 1.0f); // Adjust threshold as needed

        // Interpolate between the rotations
        FQuat BlendedQuat = FQuat::Slerp(FQuat::Slerp(LookAtQuatX, LookAtQuatY, BlendFactorY), LookAtQuatZ, BlendFactorZ);

        // Apply the initial rotation offset to maintain the correct orientation
        FRotator LookAtRotation = BlendedQuat.Rotator();
        FRotator NewRotation = LookAtRotation + InitialRotation;

        // Check if the X-axis needs to be adjusted by 180 degrees
        if (FMath::Abs(NewRotation.Pitch) > 90.0f)
        {
            NewRotation.Roll += 180.0f;
            NewRotation.Pitch = 180.0f - NewRotation.Pitch;
            NewRotation.Yaw += 180.0f;
        }

        ParentComponent->SetWorldRotation(NewRotation);
    }
}


FRotator UPuzzlePiecesComponent::AlignEulerToVectorFixedPivot(
    const FRotator& InputRotation,
    const FVector& Vector,
    float Factor,
    FVector LocalMainAxis,
    FVector LocalPivotAxis)
{
    if (LocalMainAxis.Equals(LocalPivotAxis))
    {
        return InputRotation;
    }

    if (Vector.IsNearlyZero())
    {
        return InputRotation;
    }

    FMatrix OldRotationMatrix = FRotationMatrix(InputRotation);
    FVector OldAxis = OldRotationMatrix.TransformVector(LocalMainAxis);
    FVector PivotAxis = OldRotationMatrix.TransformVector(LocalPivotAxis);

    float FullAngle = FMath::Acos(FVector::DotProduct(OldAxis, Vector.GetSafeNormal()));
    if (FullAngle > PI)
    {
        FullAngle -= 2.0f * PI;
    }
    float Angle = Factor * FullAngle;

    FQuat RotationQuat = FQuat(PivotAxis, Angle);
    FMatrix RotationMatrix = FRotationMatrix::Make(RotationQuat);

    FMatrix NewRotationMatrix = RotationMatrix * OldRotationMatrix;
    return NewRotationMatrix.Rotator();
}

FRotator UPuzzlePiecesComponent::AlignEulerToVectorAuto(
    const FRotator& InputRotation,
    const FVector& Vector,
    float Factor,
    FVector LocalMainAxis)
{
    if (Vector.IsNearlyZero())
    {
        return InputRotation;
    }

    FMatrix OldRotationMatrix = FRotationMatrix(InputRotation);
    FVector OldAxis = OldRotationMatrix.TransformVector(LocalMainAxis);

    FVector NewAxis = Vector.GetSafeNormal();
    FVector RotationAxis = FVector::CrossProduct(OldAxis, NewAxis).GetSafeNormal();
    if (RotationAxis.IsNearlyZero())
    {
        RotationAxis = FVector::CrossProduct(OldAxis, FVector(1, 0, 0)).GetSafeNormal();
        if (RotationAxis.IsNearlyZero())
        {
            RotationAxis = FVector::CrossProduct(OldAxis, FVector(0, 1, 0)).GetSafeNormal();
        }
    }

    float FullAngle = FMath::Acos(FVector::DotProduct(OldAxis, NewAxis));
    float Angle = Factor * FullAngle;

    FQuat RotationQuat = FQuat(RotationAxis, Angle);
    FMatrix RotationMatrix = FRotationMatrix::Make(RotationQuat);

    FMatrix NewRotationMatrix = RotationMatrix * OldRotationMatrix;
    return NewRotationMatrix.Rotator();
}