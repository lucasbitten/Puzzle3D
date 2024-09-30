// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/TimelineComponent.h"
#include "PuzzlePieceParentComponent.h"
#include "GI_Puzzle.h"
#include "GameFramework/SpringArmComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "PuzzlePawn.generated.h"

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

protected:

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Camera")
	USpringArmComponent* CameraSpringArm;

	UPROPERTY(BlueprintReadOnly)
	UGI_Puzzle* PuzzleGameInstance;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	USceneComponent* PiecesBoard;

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




private:

	void OnLeftMouseButtonPressed();
	void OnLeftMouseButtonReleased();
	bool CreateRayFromMouseLocation(FVector& RayStart, FVector& RayEnd);
	void HandleOnBoardPress();
	void HandleOnWorldPress();

	float CalculateDistanceFromCurrentPosition();

	void DeselectPieceComponent();


	UPuzzlePieceParentComponent* CurrentPieceComponent;
	UPuzzlePieceParentComponent* LastGrabbedPieceComponent;

	float CorrectPositionTolerance = 3;

	UMaterialInstance* OriginalMaterial;

	FVector InitialWorldLerpLocation2;

	FTimeline PlacingPieceTimeline;
	FTimeline PlacingPieceWithOffsetTimeline;


	UPROPERTY(EditAnywhere, Category = "Timeline")
	UCurveFloat* TimelineCurve;

	UFUNCTION()
	void UpdatePlacingPiece(float alpha);

	UFUNCTION()
	void UpdatePlacingPieceWithOffset(float alpha);

	UFUNCTION()
	void OnTimelineFinished();

};
