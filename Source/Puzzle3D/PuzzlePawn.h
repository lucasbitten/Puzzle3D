// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/TimelineComponent.h"
#include "PuzzlePieceParentComponent.h"
#include "GI_Puzzle.h"
#include "GameFramework/SpringArmComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PuzzlePawn.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnPieceSelected, UPuzzlePieceParentComponent*, piece);


UCLASS()
class PUZZLE3D_API APuzzlePawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	APuzzlePawn();




	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPieceSelected OnPieceSelected;

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* CameraSpringArm;

	UPROPERTY(BlueprintReadOnly)
	UGI_Puzzle* PuzzleGameInstance;

	UPROPERTY()
	float CameraMovementSpeed = 300.0f;

	UPROPERTY()
	float CameraMovementDirection = 0.0f;

	UFUNCTION(BlueprintCallable)
	void SetCameraCurrentPosition();

	void SetCameraMovementDirection(float value);

	UFUNCTION(BlueprintCallable)
	void SetGrabMode(bool grabMode);

	UFUNCTION(BlueprintCallable)
	bool GetGrabMode();

	UPROPERTY(BlueprintReadOnly)
	bool GrabMode;

	UPROPERTY(BlueprintReadWrite)
	UPuzzlePieceParentComponent* CurrentPieceComponent;

	UPROPERTY(BlueprintReadWrite)
	float CorrectPositionTolerance = 15;

	UPROPERTY(BlueprintReadWrite)
	UMaterialInterface* OriginalMaterial;

	UPROPERTY(BlueprintReadWrite)
	UMaterialInstanceDynamic* HintMaterial;

	UPROPERTY(BlueprintReadWrite)
	float PieceInWorldDistanceFromCamera = 80;

	UPROPERTY(BlueprintReadWrite)
	bool CanMovePiece = true;

	UPROPERTY(BlueprintReadWrite)
	FVector RemovingFinalPosition;

private:

	void OnLeftMouseButtonPressed();
	void OnLeftMouseButtonReleased();

	void OnRightMouseButtonPressed();
	void OnRightMouseButtonReleased();

	bool CreateRayFromMouseLocation(FVector& RayStart, FVector& RayEnd);
	void HandleOnWorldPress();

	void HandlePieceTouched(FHitResult& HitResult);
	bool IsPieceCloseToCamera();

	float CalculateDistanceFromCurrentPosition();

	void DeselectPieceComponent();



	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* TimelineCurve;

	UFUNCTION()
	void OnLerpToCorrectPositionCompletedCallback();

	UFUNCTION()
	void OnPieceLerpCloseToCameraCompletedCallback();

};
