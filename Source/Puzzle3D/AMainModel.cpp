// Fill out your copyright notice in the Description page of Project Settings.


#include "AMainModel.h"

// Sets default values for this component's properties
UAMainModel::UAMainModel()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UAMainModel::BeginPlay()
{
	UE_LOG(LogTemp, Warning, TEXT("Mensagem de aviso: Isso é um exemplo de log no console."));

	Super::BeginPlay();

	ModelActorReference = GetOwner();
	Explode();
	// ...
	
}


// Called every frame
void UAMainModel::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UAMainModel::Explode()
{
	TArray<UStaticMeshComponent*> MeshComponents;
	ModelActorReference->GetComponents<UStaticMeshComponent>(MeshComponents);
	auto ActorPos = ModelActorReference->GetActorLocation();
	int skippedPieces = 0;
	for (UStaticMeshComponent* MeshComponent : MeshComponents)
	{
		if (MeshComponent)
		{
			if (skippedPieces < InitialPieces)
			{
				skippedPieces++;
				continue;
			}

			// Calcula uma nova posição aleatória dentro da esfera
			FVector NewPosition;
			GetRandomPointInSphere(NewPosition, ActorPos);

			// Define a nova posição para o componente Static Mesh
			MeshComponent->SetWorldLocation(NewPosition);
		}
	}
}


void UAMainModel::GetRandomPointInSphere(FVector& OutPosition, const FVector& Center)
{
	// Gera coordenadas aleatórias na esfera usando coordenadas esféricas
	float Phi = FMath::FRandRange(0.0f, 2 * PI);
	float Theta = FMath::FRandRange(0.0f, PI);

	float X = Center.X + ExplosionRadius * FMath::Sin(Theta) * FMath::Cos(Phi);
	float Y = Center.Y + ExplosionRadius * FMath::Sin(Theta) * FMath::Sin(Phi);
	float Z = Center.Z + ExplosionRadius * FMath::Cos(Theta);

	OutPosition = FVector(X, Y, Z);
}
