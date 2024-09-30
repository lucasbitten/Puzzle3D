// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PuzzlePiecesComponent.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PuzzlePieceParentComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PUZZLE3D_API UPuzzlePieceParentComponent : public USceneComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UPuzzlePieceParentComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

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

	UFUNCTION(BlueprintCallable)
	const bool GetIsOnBoard() const;

	UFUNCTION(BlueprintCallable)
	const void SetIsOnBoard(bool isOnBoard);

	void SetOffsetDistance(float offset);

	UFUNCTION(BlueprintCallable)
	float GetOffsetDistance();

	void StartLerpingToCorrectPositionWithOffset();

	UFUNCTION(BlueprintCallable)
	UPuzzlePiecesComponent* GetPieceMesh() const;

private:
		
	UPROPERTY(VisibleAnywhere, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FVector InitialParentWorldPosition;

	UPROPERTY(VisibleAnywhere, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FRotator InitialParentWorldRotator;

	UPROPERTY(VisibleAnywhere, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FVector InitialParentWorldPositionWithOffset;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	bool IsLocked;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	bool IsShell;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool CanLockPieces;

	UPROPERTY(meta = (ToolTip = "Distance from shell when moving the piece"))
	float OffsetDistance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Board", meta = (AllowPrivateAccess = "true"))
	bool IsOnBoard;

	UPuzzlePiecesComponent* PuzzlePiecesComponent;


	void SetPieceMesh();

#pragma region Lerps

	FVector LerpStartPosition;
	FVector LerpEndPosition;
	float LerpPositionAlpha = 0.0f;
	float LerpPositionSpeed = 1.0f;

	FRotator LerpStartRotation;
	FRotator LerpEndRotation;
	float LerpRotationAlpha = 0.0f;
	float LerpRotationSpeed = 1.0f;

	bool IsLerping = false;

#pragma endregion

};
