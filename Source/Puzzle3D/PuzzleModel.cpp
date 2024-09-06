// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzleModel.h"
#include "Engine/StaticMeshActor.h"
#include <Kismet/KismetMathLibrary.h>
#include "InnerMesh.h"

// Sets default values
APuzzleModel::APuzzleModel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APuzzleModel::BeginPlay()
{
	Super::BeginPlay();
	GetComponents(InnerMeshComponents);
	GetComponents(PuzzlePiecesComponents);
	TotalPieces = PuzzlePiecesComponents.Num() - 1; //Excluding the shell
	OnModelInitialized.Broadcast();
	SetupPieces();
	Explode();

}

// Called every frame
void APuzzleModel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	if (ShowDebug)
	{
		DrawDebugWiredSphere();
	}
}

TArray<UInnerMesh*> APuzzleModel::GetInnerMeshComponents() const
{
	return InnerMeshComponents;
}

void APuzzleModel::SetupPieces()
{
	if (Shell == nullptr)
	{
		for (UPuzzlePiecesComponent* PuzzlePiece : PuzzlePiecesComponents)
		{
			if (PuzzlePiece != nullptr)
			{
				if (PuzzlePiece->GetIsShell())
				{
					Shell = PuzzlePiece;
					break;
				}
			}
		}
	}

	TArray<USceneComponent*> PiecesToExplode = TArray<USceneComponent*>();
	for (UPuzzlePiecesComponent* PuzzlePiece : PuzzlePiecesComponents)
	{
		if (PuzzlePiece != nullptr)
		{

			if (PuzzlePiece->GetIsShell())
			{
				continue;
			}


			DrawDebugDirectionalArrow(GetWorld(), PuzzlePiece->GetComponentLocation(), PuzzlePiece->GetComponentLocation() + -PuzzlePiece->GetAttachParent()->GetForwardVector() * 25, 5, FColor::Blue, true, 1.0f, 0, 1.0f);

			PuzzlePiece->SetParentInitialWorldPosition(PuzzlePiece->GetAttachParent()->GetComponentLocation());
			PuzzlePiece->SetParentInitialWorldRotator(PuzzlePiece->GetAttachParent()->GetComponentRotation());

			PuzzlePiece->SetOffsetDistance(OffsetDistance);
			PuzzlePiece->SetParentInitialWorldPositionWithOffset(PuzzlePiece->GetAttachParent()->GetComponentLocation() + (PuzzlePiece->GetAttachParent()->GetForwardVector() * -OffsetDistance));
		}
	}


}

void APuzzleModel::Explode()
{
	auto ActorPos = GetActorLocation();
	int skippedPieces = 0;

	TArray<USceneComponent*> PiecesToExplode = TArray<USceneComponent*>();
	for (UPuzzlePiecesComponent* PuzzlePiece : PuzzlePiecesComponents)
	{
		if (PuzzlePiece != nullptr)
		{

			if (PuzzlePiece->GetIsShell())
			{
				continue;
			}

			if (skippedPieces < InitialPieces)
			{
				skippedPieces++;
				PuzzlePiece->SetIsLocked(true);
				PuzzlePiece->GetAttachParent()->SetWorldLocation(PuzzlePiece->GetParentInitialWorldPosition());
				PuzzlePiece->GetAttachParent()->SetWorldRotation(PuzzlePiece->GetParentInitialWorldRotator());
				continue;
			}

			PuzzlePiece->SetCanLockPieces(CanLockPieces);
			PuzzlePiece->SetIsLocked(false);
			PiecesToExplode.Add(PuzzlePiece->GetAttachParent());
		}
	}

	float PiecesByCircle = 360 / DegreeSpaceBetweenPieces;
	int CircleCount = UKismetMathLibrary::FCeil(PiecesToExplode.Num() / PiecesByCircle);
	ShuffleArray(PiecesToExplode);
	int currentCircle = 0;
	float currentAngle = 0;
	for (USceneComponent* PieceParent : PiecesToExplode)
	{
		float XPos = ExplosionRadius * UKismetMathLibrary::DegSin(currentAngle);
		float YPos = ExplosionRadius * UKismetMathLibrary::DegCos(currentAngle);

		PieceParent->SetWorldLocation(FVector(XPos, YPos, GetActorLocation().Z + SpaceBetweenCircles * currentCircle - 300));
		currentAngle += DegreeSpaceBetweenPieces;

		if (currentAngle > 360)
		{
			currentCircle++;
			currentAngle = 0;
		}
	}

}

const int APuzzleModel::GetTotalPieces() const
{
	return TotalPieces;
}

const int APuzzleModel::GetInitialPieces() const
{
	return InitialPieces;
}

const float APuzzleModel::GetOffsetDistance() const
{
	return OffsetDistance;
}

const void APuzzleModel::SetInitialPieces(int32 pieces)
{
	InitialPieces = pieces;
}


void APuzzleModel::DrawDebugWiredSphere() const
{
	FVector Center = GetActorLocation(); // Center of the sphere
	int32 Segments = 24;                 // Number of segments for the wireframe
	bool bPersistentLines = false;       // Whether the lines should persist (set to true if you want them to persist)
	float LifeTime = 0.01f;              // How long the lines should last (negative value means infinite)
	uint8 DepthPriority = 0;             // Depth priority

	// Draw the debug sphere
	DrawDebugSphere(GetWorld(), Center, ExplosionRadius, Segments, FColor::Green, bPersistentLines, LifeTime, DepthPriority);
	DrawDebugSphere(GetWorld(), Center, InnerRadius, Segments, FColor::Blue, bPersistentLines, LifeTime, DepthPriority);
}


FVector APuzzleModel::GetClosestInnerMeshPoint(FVector piecePosition)
{
	FVector PointToLookAt;
	float MinDistance = FLT_MAX;

	for (UInnerMesh* InnerMesh : InnerMeshComponents)
	{
		FVector StartLocation = piecePosition;
		FVector EndLocation = InnerMesh->GetComponentLocation();

		FHitResult HitResult;
		FCollisionQueryParams CollisionParams;
		CollisionParams.bTraceComplex = true;

		// Realiza o Line Trace
		bool bHit = GetWorld()->LineTraceSingleByChannel(
			HitResult,
			StartLocation,
			EndLocation,
			ECC_GameTraceChannel2, // Canal da shell
			CollisionParams
		);

		// Se algo foi atingido
		if (bHit)
		{
			if (FVector::Distance(StartLocation, HitResult.ImpactPoint) < MinDistance)
			{
				PointToLookAt = HitResult.ImpactPoint;
				MinDistance = FVector::Distance(StartLocation, HitResult.ImpactPoint);
			}
		}
	}

	return PointToLookAt;

}

