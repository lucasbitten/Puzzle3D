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

TArray<UInnerMesh*> APuzzleModel::GetInnerMeshComponents() const
{
	return InnerMeshComponents;
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

				//// Par�metros do LineTrace
				//FVector StartLocation = NewPieceParent->GetComponentLocation();
				//FVector EndLocation = NewPieceParent->GetComponentLocation() + NewPieceParent->GetForwardVector() * 0.1f;

				//FHitResult HitResult;
				//FCollisionQueryParams CollisionParams;
				//CollisionParams.bTraceComplex = true;
				//float SphereRadius = 10.0f;

				//// Realiza o Line Trace
				//bool bHit = GetWorld()->SweepSingleByChannel(
				//	HitResult,
				//	StartLocation,
				//	EndLocation,
				//	FQuat::Identity,
				//	ECC_GameTraceChannel2, // Canal de da shell
				//	FCollisionShape::MakeSphere(SphereRadius),
				//	CollisionParams
				//);

				//FVector pointToLook;

				//// Se algo foi atingido
				//if (bHit)
				//{
				//	pointToLook = HitResult.ImpactPoint;
				//}


				//auto pointToLook = CalculateWeightedAverage(PuzzlePiece->GetComponentLocation());
				//InitialRotator = UKismetMathLibrary::FindLookAtRotation(NewPieceParent->GetComponentLocation(), pointToLook);

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

				// Calcula uma nova posi��o aleat�ria dentro da esfera
				FVector NewPosition;
				GetRandomPointInSphere(NewPosition, ActorPos);

				// Define a nova posi��o para o componente Static Mesh
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
	// Calcula a diferen�a de raio entre a esfera externa e a esfera interna
	float EffectiveRadius = ExplosionRadius - InnerRadius;

	// Gera coordenadas aleat�rias na esfera usando coordenadas esf�ricas
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

FVector APuzzleModel::CalculateWeightedAverage(FVector piecePosition)
{
	// Vari�veis para armazenar as somas
	float TotalWeight = 0.0f;
	float SumX = 0.0f;
	float SumY = 0.0f;
	float SumZ = 0.0f;

	// Vari�veis para armazenar as coordenadas m�nimas e m�ximas
	FVector MinCoords(FLT_MAX, FLT_MAX, FLT_MAX);
	FVector MaxCoords(FLT_MIN, FLT_MIN, FLT_MIN);

	for (UInnerMesh* InnerMesh : InnerMeshComponents)
	{
		// Par�metros do LineTrace
		FVector StartLocation = piecePosition;
		FVector EndLocation = InnerMesh->GetComponentLocation();

		if (FVector::Distance(StartLocation, EndLocation) > MaxRaycastLength)
		{
			continue;
		}

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
			FVector ImpactPoint = HitResult.ImpactPoint; // Ponto de impacto
			float DistancePower = 10.0f;

			// Calcular a dist�ncia
			float Distance = FMath::Max(HitResult.Distance, 1.0f);

			// Aplicar uma pot�ncia � dist�ncia
			float WeightedDistance = FMath::Pow(Distance, DistancePower);

			// Calcular o peso
			float Weight = 1.0f / FMath::Max(WeightedDistance, 0.0001f); // Evitar divis�o por zero

			// Atualizar as somas ponderadas
			TotalWeight += Weight;
			SumX += Weight * ImpactPoint.X;
			SumY += Weight * ImpactPoint.Y;
			SumZ += Weight * ImpactPoint.Z;

			// Atualizar os valores m�nimos e m�ximos das coordenadas
			MinCoords.X = FMath::Min(MinCoords.X, ImpactPoint.X);
			MinCoords.Y = FMath::Min(MinCoords.Y, ImpactPoint.Y);
			MinCoords.Z = FMath::Min(MinCoords.Z, ImpactPoint.Z);

			MaxCoords.X = FMath::Max(MaxCoords.X, ImpactPoint.X);
			MaxCoords.Y = FMath::Max(MaxCoords.Y, ImpactPoint.Y);
			MaxCoords.Z = FMath::Max(MaxCoords.Z, ImpactPoint.Z);

			// Opcional: Desenhar a linha de rastreamento para visualiza��o no editor
			DrawDebugLine(GetWorld(), StartLocation, EndLocation, FColor::Green, false, 1, 0, 1);
		}
	}

	// Dividir as somas pelo peso total para obter o ponto m�dio ponderado
	FVector WeightedAverage;
	if (TotalWeight > 0.0f)
	{
		WeightedAverage.X = SumX / TotalWeight;
		WeightedAverage.Y = SumY / TotalWeight;
		WeightedAverage.Z = SumZ / TotalWeight;
	}
	else
	{
		WeightedAverage = FVector::ZeroVector; // Default no caso de n�o haver peso acumulado
	}

	// Aplicar o Clamp para garantir que o ponto esteja dentro do intervalo
	WeightedAverage.X = FMath::Clamp(WeightedAverage.X, MinCoords.X, MaxCoords.X);
	WeightedAverage.Y = FMath::Clamp(WeightedAverage.Y, MinCoords.Y, MaxCoords.Y);
	WeightedAverage.Z = FMath::Clamp(WeightedAverage.Z, MinCoords.Z, MaxCoords.Z);

	return WeightedAverage;
}

