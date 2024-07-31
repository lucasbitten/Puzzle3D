// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PuzzlePiecesComponent.h"
#include "PuzzleModel.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnModelLoaded);


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
	FOnModelLoaded OnModelLoaded;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool ShowDebug;

	UPROPERTY(EditAnywhere, Category = "Debug")
	bool CanLockPieces;

private:
	UFUNCTION(BlueprintCallable)
	void Explode();
	void GetRandomPointInSphere(FVector& OutPosition, const FVector& Center);
	TArray<UPuzzlePiecesComponent*> PuzzlePiecesComponents;

	void DrawDebugWiredSphere() const;

	UPROPERTY(EditAnywhere, Category = "Move Info")
	float OffsetDistance = 20.0;

};
