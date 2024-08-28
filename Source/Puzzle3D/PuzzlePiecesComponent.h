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

	void SetParentInitialWorldPosition(FVector initialPos);
	UFUNCTION(BlueprintCallable)
	const FVector GetParentInitialWorldPosition() const;

	void SetParentInitialWorldPositionWithOffset(FVector initialPos);
	UFUNCTION(BlueprintCallable)
	const FVector GetParentInitialWorldPositionWithOffset() const;

	void SetParentInitialWorldRotator(FRotator initialRotation);
	UFUNCTION(BlueprintCallable)
	const FRotator GetParentInitialWorldRotator() const;


	UFUNCTION(BlueprintCallable)
	const bool GetIsLocked() const;
	UFUNCTION(BlueprintCallable)
	const void SetIsLocked(bool locked);


	const void SetCanLockPieces(bool canLock);


	const bool GetIsShell() const;

	const void SetIsShell(bool isShell);

	void SetOffsetDistance(float offset);

	UFUNCTION(BlueprintCallable)
	float GetOffsetDistance();

private:
	UPROPERTY(VisibleAnywhere, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FVector InitialParentWorldPosition;

	UPROPERTY(VisibleAnywhere, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FRotator InitialParentWorldRotator;

	UPROPERTY(VisibleAnywhere, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FVector InitialParentWorldPositionWithOffset;


	UPROPERTY(VisibleAnywhere, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FVector ShellRelativePosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	bool IsLocked;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	bool IsShell;


	UPROPERTY(EditAnywhere, Category = "Debug")
	bool CanLockPieces;


	float OffsetDistance;

	UMaterialInstanceDynamic* DynamicMaterial;
	UMaterialInterface* InitialMaterial;

};
