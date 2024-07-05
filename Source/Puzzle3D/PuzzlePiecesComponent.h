// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "PuzzlePiecesComponent.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLE3D_API UPuzzlePiecesComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

public:

	void SetParentInitialRelativePosition(FVector initialPos);
	UFUNCTION(BlueprintCallable)
	const FVector GetParentInitialRelativePosition() const;

	void SetParentInitialRelativeRotator(FRotator initialRotation);
	UFUNCTION(BlueprintCallable)
	const FRotator GetParentInitialRelativeRotator() const;




	// Function to calculate and store rotation offset based on parent component
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void CalculateRotationOffset();

	// Function to get the stored rotation offset
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	FRotator GetRotationOffset() const;



	void SetShellRelativePosition(FVector position);
	UFUNCTION(BlueprintCallable)
	const FVector GetShellRelativePosition() const;



	UFUNCTION(BlueprintCallable)
	const bool GetIsLocked() const;
	UFUNCTION(BlueprintCallable)
	const void SetIsLocked(bool locked);


	const void SetCanLockPieces(bool canLock);


	const bool GetIsShell() const;

	const void SetIsShell(bool isShell);

private:
	UPROPERTY(VisibleAnywhere, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FVector InitialParentRelativePosition;

	UPROPERTY(VisibleAnywhere, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FRotator InitialParentRelativeRotator;

	UPROPERTY(VisibleAnywhere, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FQuat InitialQuat;


	UPROPERTY(VisibleAnywhere, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FVector ShellRelativePosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	bool IsLocked;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	bool IsShell;

	// Function to calculate and store initial rotation
	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void CalculateInitialRotation();

	UFUNCTION(BlueprintCallable, Category = "Puzzle")
	void MoveParentToSurface(USceneComponent* ParentComponent, FVector ImpactPoint, FVector ImpactNormal, float Offset);


	UPROPERTY(EditAnywhere, Category = "Debug")
	bool CanLockPieces;

	FRotator RotationOffset;
	FRotator InitialRotation;


};
