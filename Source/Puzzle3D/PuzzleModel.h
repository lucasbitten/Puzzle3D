// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GM_PuzzleMode.h"
#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzlePiecesComponent.h"
#include "PuzzlePieceParentComponent.h"
#include "PuzzleModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnPiecePlaced);


class UInnerMesh;



UCLASS()
class PUZZLE3D_API APuzzleModel : public AActor
{
	GENERATED_BODY()

 public:	
	// Sets default values for this actor's properties
	APuzzleModel();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components")
	USceneComponent* ScreenSidePosition;


	UPROPERTY(EditAnywhere, Category = "Components")
	float SidePiecesDistanceFromScreen = 0.1f;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;


	UFUNCTION(BlueprintCallable, Category = "Mesh Groups")
	TArray<UInnerMesh*> GetInnerMeshComponents() const;

	UFUNCTION(BlueprintCallable, Category = "Board")
	TArray<UPuzzlePieceParentComponent*> GetPiecesToSendToBoard() const;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float DegreeSpaceBetweenPieces = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float SpaceBetweenCircles = 20;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float ExplosionRadius = 100;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	int32 InitialPieces = 50;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float PiecesScaleFactor = 1;


	UFUNCTION(BlueprintCallable)
	const int GetTotalPieces() const;

	UFUNCTION(BlueprintCallable)
	const int GetCorrectPiecesPlaced() const;

	UFUNCTION(BlueprintCallable)
	const int GetInitialPieces() const;

	UFUNCTION(BlueprintCallable)
	const void SetInitialPieces(int32 pieces);

	UFUNCTION(BlueprintCallable)
	const float GetOffsetDistance() const;

	UFUNCTION()
	void OnPieceSelected(UPuzzlePieceParentComponent* piece);


	void OnPiecePlaced(UPuzzlePieceParentComponent* piece);

	UPROPERTY(EditAnywhere, Category = "Piece Movement")
	bool ShowDebug;

	UPROPERTY(EditAnywhere, Category = "Piece Movement")
	bool CanLockPieces = true;

	UPROPERTY(EditAnywhere, Category = "Piece Movement")
	float MaxRaycastLength = 25;

	UPROPERTY(BlueprintReadOnly)
	UPuzzlePieceParentComponent* Shell;

	UPROPERTY(BlueprintReadOnly)
	float ModelTopZ;	
	
	UPROPERTY(BlueprintReadOnly)
	float ModelBottomZ;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnPiecePlaced OnPiecePlacedEvent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Audio")
	USoundBase* Fitting;


	void PlayFittingSound() const;

private:

	UPROPERTY(EditAnywhere,Category = "Materials")
	TArray<UMaterialInterface*> PieceAlwaysOnTopMaterials;
	UPROPERTY(EditAnywhere, Category = "Materials")
	TArray<UMaterialInterface*> PieceDefaultMaterials;


	void InitializeAlwaysOnTopMaterials();

	void SetPieceMaterial(UStaticMeshComponent* Piece, bool bAlwaysOnTop);


	void SetScreenSidePosition();


	void SetupModel();

	UFUNCTION(BlueprintCallable)
	void Explode();

	void MovePiecesToScreenSide();

	void MovePiecesToCylinder();

	UFUNCTION(BlueprintCallable)
	void ResetModel();

	AGM_PuzzleMode* PuzzleMode;

	TArray<UPuzzlePieceParentComponent*> PuzzlePieceParentComponents;

	UPROPERTY(EditAnywhere, Category = "Piece Movement", meta = (AllowPrivateAccess = "true", ToolTip = "Distance from shell when moving the piece"))
	float OffsetDistance = 2.0;

	TArray<UInnerMesh*> InnerMeshComponents;

	TArray<UPuzzlePieceParentComponent*> PiecesToSendToBoard;

	int TotalPieces;

	int CorrectPiecesPlaced;

	void SaveAllPieces();

	void SavePiece(FString PieceID, bool IsCorrectlyPositioned);

	void LoadAllPieces();

	void LoadPiece(FString PieceID);

	UPuzzlePieceParentComponent* FindPieceByIdentifier(FString PieceID);

	bool loadedFromSaveGame;


};

template <typename T>
void ShuffleArray(TArray<T>& Array)
{
	if (Array.Num() > 0)
	{
		int32 LastIndex = Array.Num() - 1;
		for (int32 i = 0; i <= LastIndex; ++i)
		{
			int32 Index = FMath::RandRange(i, LastIndex);
			if (i != Index)
			{
				Array.Swap(i, Index);
			}
		}
	}
}