// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzlePiecesComponent.h"
#include "PuzzleModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnModelLoaded);

class UInnerMesh;

UENUM(BlueprintType)
enum class NormalCalculationType : uint8
{
	CalculateWeightedAverage UMETA(DisplayName = "CalculateWeightedAverage"),
	GetClosestInnerMeshPoint UMETA(DisplayName = "GetClosestInnerMeshPoint"),
	GetPointToLookAtFromSphereCast UMETA(DisplayName = "GetPointToLookAtFromSphereCast")
};

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



public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float DegreeSpaceBetweenPieces;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float SpaceBetweenCircles;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float ExplosionRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float InnerRadius;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	int32 InitialPieces = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Explosion")
	int TotalPieces;

	UFUNCTION(BlueprintCallable)
	const int GetTotalPieces() const;

	UFUNCTION(BlueprintCallable)
	const int GetInitialPieces() const;

	UFUNCTION(BlueprintCallable)
	const void SetInitialPieces(int32 pieces);

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnModelLoaded OnModelInitialized;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool ShowDebug;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool CanLockPieces;

	UPROPERTY(EditAnywhere, Category = "Debug")
	float SphereCollisionRadius = 5.0f;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool GetPreviousDirectionAverage;


	UFUNCTION(BlueprintCallable, Category = "Mesh Groups")
	TArray<UInnerMesh*> GetInnerMeshComponents() const;

	UPROPERTY(EditAnywhere, Category = "Raycast")
	float MaxRaycastLength = 250;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Debug")
	NormalCalculationType NormalCalculationType;


private:

	UPuzzlePiecesComponent* Shell;

	void SetupPieces();

	UFUNCTION(BlueprintCallable)
	void Explode();
	TArray<UPuzzlePiecesComponent*> PuzzlePiecesComponents;

	void DrawDebugWiredSphere() const;

	UPROPERTY(EditAnywhere, Category = "Move Info")
	float OffsetDistance = 20.0;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Mesh Groups", meta = (AllowPrivateAccess = "true"))
	TArray<UInnerMesh*> InnerMeshComponents;

	FVector GetClosestInnerMeshPoint(FVector piecePosition);

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