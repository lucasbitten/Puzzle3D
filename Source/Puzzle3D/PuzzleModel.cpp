// Fill out your copyright notice in the Description page of Project Settings.

#include "PuzzleModel.h"
#include "Engine/StaticMeshActor.h"
#include <Kismet/KismetMathLibrary.h>
#include "InnerMesh.h"
#include "PuzzleSaveGame.h" 
#include <Kismet/GameplayStatics.h>

// Sets default values
APuzzleModel::APuzzleModel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;
}


void APuzzleModel::SaveAllPieces()
{
	UPuzzleSaveGame* SaveGameInstance = Cast<UPuzzleSaveGame>(UGameplayStatics::CreateSaveGameObject(UPuzzleSaveGame::StaticClass()));
	SaveGameInstance->SaveAllPiecesState("EsculturaExemplo", PuzzlePieceParentComponents);
}

// Função para salvar o estado de uma peça específica
void APuzzleModel::SavePiece(FString PieceID, bool IsCorrectlyPositioned)
{
	UPuzzleSaveGame* SaveGameInstance = Cast<UPuzzleSaveGame>(UGameplayStatics::CreateSaveGameObject(UPuzzleSaveGame::StaticClass()));

	// Chama o método de salvar a peça específica, passando o PieceID (identifier)
	SaveGameInstance->SavePieceState("EsculturaExemplo", PieceID, IsCorrectlyPositioned);
}

void APuzzleModel::LoadAllPieces()
{
	UPuzzleSaveGame* SaveGameInstance = Cast<UPuzzleSaveGame>(UGameplayStatics::CreateSaveGameObject(UPuzzleSaveGame::StaticClass()));

	TArray<FPuzzlePieceSaveData> LoadedPieces = SaveGameInstance->LoadAllPiecesState("EsculturaExemplo");

	// Agora você pode atualizar as peças no seu modelo com os dados carregados
	for (int i = 0; i < LoadedPieces.Num(); i++)
	{
		UPuzzlePieceParentComponent* PuzzlePiece = FindPieceByIdentifier(LoadedPieces[i].PieceID);

		if (PuzzlePiece)
		{
			// Atualiza a peça com o estado carregado
			PuzzlePiece->SetIsLocked(LoadedPieces[i].IsCorrectlyPositioned);
		}
	}
}

// Função para carregar o estado de uma peça específica
void APuzzleModel::LoadPiece(FString PieceID)
{
	UPuzzleSaveGame* SaveGameInstance = Cast<UPuzzleSaveGame>(UGameplayStatics::CreateSaveGameObject(UPuzzleSaveGame::StaticClass()));

	bool IsCorrectlyPositioned = false;
	bool bLoaded = SaveGameInstance->LoadPieceState("EsculturaExemplo", PieceID, IsCorrectlyPositioned);

	if (bLoaded)
	{
		// Procura a peça pelo identifier no array de peças
		UPuzzlePieceParentComponent* PuzzlePiece = FindPieceByIdentifier(PieceID);

		if (PuzzlePiece)
		{
			// Atualiza o estado da peça com o valor carregado
			PuzzlePiece->SetIsLocked(IsCorrectlyPositioned); // Ajuste de acordo com sua lógica
		}
	}
}


UPuzzlePieceParentComponent* APuzzleModel::FindPieceByIdentifier(FString PieceID)
{
	for (UPuzzlePieceParentComponent* Piece : PuzzlePieceParentComponents)
	{
		if (Piece && Piece->GetIdentifier() == PieceID)
		{
			return Piece;  // Retorna a peça encontrada
		}
	}

	return nullptr;  // Retorna nullptr se não encontrar a peça
}


// Called when the game starts or when spawned
void APuzzleModel::BeginPlay()
{
	Super::BeginPlay();
	GetComponents(InnerMeshComponents);
	GetComponents(PuzzlePieceParentComponents);
	TotalPieces = PuzzlePieceParentComponents.Num() - 1; //Excluding the shell
	SetupModel();

	AGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode();
	if (GameModeBase)
	{
		PuzzleMode = Cast<AGM_PuzzleMode>(GameModeBase);

		if (PuzzleMode)
		{
			PuzzleMode->OnModelLoaded.Broadcast();
		}

	}
	LoadAllPieces();
	Explode();
	SaveAllPieces();
}

// Called every frame
void APuzzleModel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

TArray<UInnerMesh*> APuzzleModel::GetInnerMeshComponents() const
{
	return InnerMeshComponents;
}

TArray<USceneComponent*> APuzzleModel::GetPiecesToSendToBoard() const
{
	return PiecesToSendToBoard;
}

void APuzzleModel::SetupModel()
{
	if (Shell == nullptr)
	{
		for (UPuzzlePieceParentComponent* PuzzlePiece : PuzzlePieceParentComponents)
		{
			if (PuzzlePiece != nullptr)
			{
				if (PuzzlePiece->GetIsShell())
				{
					Shell = PuzzlePiece;

					FBoxSphereBounds Bounds = Shell->GetPieceMesh()->CalcBounds(Shell->GetPieceMesh()->GetComponentTransform());

					ModelTopZ = Bounds.Origin.Z + Bounds.BoxExtent.Z;
					ModelBottomZ = Bounds.Origin.Z - Bounds.BoxExtent.Z;

					break;
				}
			}
		}
	}

	for (UPuzzlePieceParentComponent* PuzzlePiece : PuzzlePieceParentComponents)
	{
		if (PuzzlePiece != nullptr)
		{

			if (PuzzlePiece->GetIsShell())
			{
				continue;
			}

			if (ShowDebug)
			{
				DrawDebugDirectionalArrow(GetWorld(), PuzzlePiece->GetComponentLocation(), PuzzlePiece->GetComponentLocation() + -PuzzlePiece->GetForwardVector() * 5, 0.1, FColor::Blue, true, 1.0f, 0, 0.2f);
			}


			PuzzlePiece->SetParentInitialWorldPosition(PuzzlePiece->GetComponentLocation());
			PuzzlePiece->SetParentInitialWorldRotator(PuzzlePiece->GetComponentRotation());

			PuzzlePiece->SetOffsetDistance(OffsetDistance);
			PuzzlePiece->SetParentInitialWorldPositionWithOffset(PuzzlePiece->GetComponentLocation() + (PuzzlePiece->GetForwardVector() * -OffsetDistance));
		}
	}
}

void APuzzleModel::Explode()
{
	if (Shell == nullptr)
	{
		UE_LOG(LogTemp, Error, TEXT("Shell is null!"));
		return;
	}


	auto ActorPos = GetActorLocation();
	int skippedPieces = 0;

	CorrectPiecesPlaced = 0;

	PiecesToSendToBoard.Empty();

	for (UPuzzlePieceParentComponent* PuzzlePiece : PuzzlePieceParentComponents)
	{
		if (PuzzlePiece != nullptr)
		{

			if (PuzzlePiece->GetIsShell())
			{
				continue;
			}

			PuzzlePiece->SetCanLockPieces(CanLockPieces);

			if (skippedPieces < InitialPieces || PuzzlePiece->GetIsLocked())
			{
				skippedPieces++;
				PuzzlePiece->SetIsLocked(true);
				PuzzlePiece->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
				PuzzlePiece->SetIsOnBoard(false);
				PuzzlePiece->SetWorldLocation(PuzzlePiece->GetParentInitialWorldPosition());
				PuzzlePiece->SetWorldRotation(PuzzlePiece->GetParentInitialWorldRotator());
				CorrectPiecesPlaced++;
				continue;
			}

			PuzzlePiece->SetIsLocked(false);
			PuzzlePiece->SetIsOnBoard(true);
			PiecesToSendToBoard.Add(PuzzlePiece);

		}
	}

	float PiecesByCircle = 360 / DegreeSpaceBetweenPieces;
	int CircleCount = UKismetMathLibrary::FCeil(PiecesToSendToBoard.Num() / PiecesByCircle);
	ShuffleArray(PiecesToSendToBoard);
	int currentCircle = 0;
	float currentAngle = 0;
	for (USceneComponent* PieceParent : PiecesToSendToBoard)
	{
		float XPos = ExplosionRadius * UKismetMathLibrary::DegSin(currentAngle);
		float YPos = ExplosionRadius * UKismetMathLibrary::DegCos(currentAngle);

		PieceParent->SetWorldLocation(FVector(XPos, YPos, GetActorLocation().Z + SpaceBetweenCircles * currentCircle - 30));
		currentAngle += DegreeSpaceBetweenPieces;

		if (currentAngle > 360)
		{
			currentCircle++;
			currentAngle = 0;
		}

		FRotator Rotator = UKismetMathLibrary::FindLookAtRotation(PieceParent->GetComponentLocation(), Shell->GetComponentLocation());
		PieceParent->SetWorldRotation(Rotator);
		
	}

	if (PuzzleMode)
	{
		PuzzleMode->OnModelExploded.Broadcast();
	}
}

const int APuzzleModel::GetTotalPieces() const
{
	return TotalPieces;
}

const int APuzzleModel::GetCorrectPiecesPlaced() const
{
	return CorrectPiecesPlaced;
}

const int APuzzleModel::GetInitialPieces() const
{
	return InitialPieces;
}

const float APuzzleModel::GetOffsetDistance() const
{
	return OffsetDistance;
}

void APuzzleModel::OnPiecePlaced(UPuzzlePieceParentComponent* piece)
{
	CorrectPiecesPlaced++;
	OnPiecePlacedEvent.Broadcast();
	SaveAllPieces();
}

const void APuzzleModel::SetInitialPieces(int32 pieces)
{
	InitialPieces = pieces;
}


