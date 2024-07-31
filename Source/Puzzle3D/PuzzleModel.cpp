// Fill out your copyright notice in the Description page of Project Settings.


#include "PuzzleModel.h"
#include "Engine/StaticMeshActor.h"
#include <Kismet/KismetMathLibrary.h>

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
	GetComponents(PuzzlePiecesComponents);
	TotalPieces = PuzzlePiecesComponents.Num() - 1; //Excluding the shell
	OnModelLoaded.Broadcast();
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

void APuzzleModel::Explode()
{
	auto ActorPos = GetActorLocation();
	int skippedPieces = 0;

	UPuzzlePiecesComponent* Shell = nullptr;
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

	for (UPuzzlePiecesComponent* PuzzlePiece : PuzzlePiecesComponents)
	{
		if (PuzzlePiece != nullptr)
		{

			if (PuzzlePiece->GetIsShell())
			{
				continue;
			}

			// Crie um novo componente de cena vazio
			FName NewComponentName = FName(*FString::Printf(TEXT("%s_Parent"), *PuzzlePiece->GetName()));
			USceneComponent* NewPieceParent = NewObject<USceneComponent>(this, USceneComponent::StaticClass(), NewComponentName);

			if (NewPieceParent)
			{
				// Anexe o novo componente de cena vazio ao root component do ator
				NewPieceParent->SetupAttachment(RootComponent);

				// Adicione e registre o novo componente
				NewPieceParent->RegisterComponent();
				AddInstanceComponent(NewPieceParent);

				PuzzlePiece->SetParentInitialWorldPosition(PuzzlePiece->GetComponentLocation());

				// Anexe o componente existente ao novo componente de cena vazio
				PuzzlePiece->AttachToComponent(NewPieceParent, FAttachmentTransformRules::KeepWorldTransform);
				FRotator InitialRotator;

				NewPieceParent->SetWorldLocation(PuzzlePiece->GetComponentLocation());
				InitialRotator = UKismetMathLibrary::FindLookAtRotation(NewPieceParent->GetComponentLocation(), Shell->GetComponentLocation());

				NewPieceParent->SetWorldRotation(InitialRotator);
				PuzzlePiece->SetParentInitialWorldRotator(NewPieceParent->GetComponentRotation());

				PuzzlePiece->SetOffsetDistance(OffsetDistance);
				PuzzlePiece->SetParentInitialWorldPositionWithOffset(NewPieceParent->GetComponentLocation() + (NewPieceParent->GetForwardVector() * -OffsetDistance));

				FRotator CurrentRotator = PuzzlePiece->GetComponentRotation();
				FRotator PieceEndRotator = UKismetMathLibrary::ComposeRotators(CurrentRotator, UKismetMathLibrary::NegateRotator(InitialRotator));

				PuzzlePiece->SetWorldRotation(PieceEndRotator);

				if (skippedPieces < InitialPieces)
				{
					PuzzlePiece->SetWorldRotation(PieceEndRotator);
					PuzzlePiece->SetRelativeLocation(FVector::Zero());
					skippedPieces++;
					PuzzlePiece->SetIsLocked(true);
					continue;
				}

				PuzzlePiece->SetCanLockPieces(CanLockPieces);
				PuzzlePiece->SetIsLocked(false);

				// Calcula uma nova posição aleatória dentro da esfera
				FVector NewPosition;
				GetRandomPointInSphere(NewPosition, ActorPos);

				// Define a nova posição para o componente Static Mesh
				NewPieceParent->SetWorldLocation(NewPosition);
				PuzzlePiece->SetRelativeLocation(FVector::Zero());
				PuzzlePiece->SetWorldRotation(PieceEndRotator);


			}
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

const void APuzzleModel::SetInitialPieces(int32 pieces)
{
	InitialPieces = pieces;
}


void APuzzleModel::GetRandomPointInSphere(FVector& OutPosition, const FVector& Center)
{
	// Calcula a diferença de raio entre a esfera externa e a esfera interna
	float EffectiveRadius = ExplosionRadius - InnerRadius;

	// Gera coordenadas aleatórias na esfera usando coordenadas esféricas
	float Phi = FMath::FRandRange(0.0f, 2 * PI);
	float Theta = FMath::FRandRange(0.0f, PI);
	float Radius = FMath::FRandRange(0.0f, EffectiveRadius) + InnerRadius;

	float X = Center.X + Radius * FMath::Sin(Theta) * FMath::Cos(Phi);
	float Y = Center.Y + Radius * FMath::Sin(Theta) * FMath::Sin(Phi);
	float Z = Center.Z + Radius * FMath::Cos(Theta);

	OutPosition = FVector(X, Y, Z);
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

