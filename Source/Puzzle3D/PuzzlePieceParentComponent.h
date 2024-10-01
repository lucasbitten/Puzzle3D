// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Components/TimelineComponent.h"
#include "PuzzlePiecesComponent.h"
#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "PuzzlePieceParentComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnLerpCompleted);


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

	FRotator PieceInitialRelativeRotation;

	UPROPERTY(EditAnywhere, Category = "Lerp")
	UCurveFloat* LerpCurve;

public:	

	void SetLerpCurve(UCurveFloat* Curve);

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

	UFUNCTION(BlueprintCallable)
	void SavePieceInitialRelativeRotation();

	UFUNCTION(BlueprintCallable)
	void RestorePieceInitialRelativeRotation();


private:

	float Delta;
		
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

	void InitializeLerpToPositionWithOffsetTimeline();

#pragma region Lerps

public:
	FOnLerpCompleted OnLerpCompletedCallback;
private:

	FTimeline LerpToPositionWithOffsetTimeline;
	FTimeline LerpToPositionTimeline;



	FVector LerpStartPosition;
	FVector LerpEndPosition;
	float LerpPositionSpeed = 1.0f;

	FRotator LerpStartRotation;
	FRotator LerpEndRotation;
	float LerpRotationSpeed = 20.0f;

	bool IsLerpingToCorrectPositionWithOffset = false;
	bool IsLerpingToCorrectPosition = false;

	UFUNCTION()
	void HandleLerpWithOffsetProgress(float Value);

	UFUNCTION()
	void HandleLerpProgress(float Value);

	UFUNCTION()
	void OnLerpToPositionWithOffsetTimelineFinished();

	UFUNCTION()
	void OnLerpToPositionTimelineFinished();

#pragma endregion

};
