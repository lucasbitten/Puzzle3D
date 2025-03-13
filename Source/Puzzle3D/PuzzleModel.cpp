// Fill out your copyright notice in the Description page of Project Settings.

#include "PuzzleModel.h"
#include "Engine/StaticMeshActor.h"
#include <Kismet/KismetMathLibrary.h>
#include "InnerMesh.h"
#include "PuzzleSaveGame.h" 
#include <Kismet/GameplayStatics.h>
#include "PuzzlePawn.h"

// Sets default values
APuzzleModel::APuzzleModel()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	if (!RootComponent)
	{
		RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("DefaultRoot"));
	}

	ScreenSidePosition = CreateDefaultSubobject<USceneComponent>(TEXT("ScreenSidePosition"));
	ScreenSidePosition->SetupAttachment(RootComponent);
	
}


void APuzzleModel::SaveAllPieces()
{
	UPuzzleSaveGame* SaveGameInstance = Cast<UPuzzleSaveGame>(UGameplayStatics::CreateSaveGameObject(UPuzzleSaveGame::StaticClass()));
	if (!SaveGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("SaveAllPieces: Failed to create SaveGame object."));
		return;
	}

	SaveGameInstance->SaveAllPiecesState(GetActorLabel(), PuzzlePieceParentComponents);

	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGameInstance, GetActorLabel(), 0);
	if (bSaved)
	{
		UE_LOG(LogTemp, Log, TEXT("SaveAllPieces: Successfully saved all pieces for sculpture '%s'."), *GetActorLabel());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SaveAllPieces: Failed to save all pieces for sculpture '%s'."), *GetActorLabel());
	}
}


void APuzzleModel::SavePiece(FString PieceID, bool IsCorrectlyPositioned)
{
	UPuzzleSaveGame* SaveGameInstance = Cast<UPuzzleSaveGame>(UGameplayStatics::CreateSaveGameObject(UPuzzleSaveGame::StaticClass()));
	if (!SaveGameInstance)
	{
		UE_LOG(LogTemp, Error, TEXT("SavePiece: Failed to create SaveGame object."));
		return;
	}

	SaveGameInstance->SavePieceState(GetActorLabel(), PieceID, IsCorrectlyPositioned);

	bool bSaved = UGameplayStatics::SaveGameToSlot(SaveGameInstance, GetActorLabel(), 0);
	if (bSaved)
	{
		UE_LOG(LogTemp, Log, TEXT("SavePiece: Successfully saved piece '%s' for sculpture '%s'."), *PieceID, *GetActorLabel());
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("SavePiece: Failed to save piece '%s' for sculpture '%s'."), *PieceID, *GetActorLabel());
	}
}

void APuzzleModel::LoadAllPieces()
{
	UPuzzleSaveGame* SaveGameInstance = Cast<UPuzzleSaveGame>(UGameplayStatics::LoadGameFromSlot(GetActorLabel(), 0));
	if (!SaveGameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadAllPieces: No SaveGame object found for sculpture '%s'. Using default state."), *GetActorLabel());
		return;
	}

	TArray<FPuzzlePieceSaveData> LoadedPieces = SaveGameInstance->LoadAllPiecesState(GetActorLabel());
	if (LoadedPieces.Num() == 0)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadAllPieces: No pieces data found for sculpture '%s'."), *GetActorLabel());
		return;
	}

	for (int i = 0; i < LoadedPieces.Num(); i++)
	{
		UPuzzlePieceParentComponent* PuzzlePiece = FindPieceByIdentifier(LoadedPieces[i].PieceID);
		if (PuzzlePiece)
		{
			PuzzlePiece->SetIsLocked(LoadedPieces[i].IsCorrectlyPositioned);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LoadAllPieces: Piece '%s' not found in the current scene."), *LoadedPieces[i].PieceID);
		}
	}

	loadedFromSaveGame = true;
}

// Fun��o para carregar o estado de uma pe�a espec�fica
void APuzzleModel::LoadPiece(FString PieceID)
{
	UPuzzleSaveGame* SaveGameInstance = Cast<UPuzzleSaveGame>(UGameplayStatics::LoadGameFromSlot(GetActorLabel(), 0));
	if (!SaveGameInstance)
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadPiece: No SaveGame object found for sculpture '%s'."), *GetActorLabel());
		return;
	}

	bool IsCorrectlyPositioned = false;
	bool bLoaded = SaveGameInstance->LoadPieceState(GetActorLabel(), PieceID, IsCorrectlyPositioned);

	if (bLoaded)
	{
		UPuzzlePieceParentComponent* PuzzlePiece = FindPieceByIdentifier(PieceID);
		if (PuzzlePiece)
		{
			PuzzlePiece->SetIsLocked(IsCorrectlyPositioned);
		}
		else
		{
			UE_LOG(LogTemp, Warning, TEXT("LoadPiece: Piece '%s' not found in the current scene."), *PieceID);
		}
	}
	else
	{
		UE_LOG(LogTemp, Warning, TEXT("LoadPiece: Failed to load piece '%s' for sculpture '%s'."), *PieceID, *GetActorLabel());
	}
}


UPuzzlePieceParentComponent* APuzzleModel::FindPieceByIdentifier(FString PieceID)
{
	for (UPuzzlePieceParentComponent* Piece : PuzzlePieceParentComponents)
	{
		if (Piece && Piece->GetIdentifier() == PieceID)
		{
			return Piece;
		}
	}

	return nullptr;
}


// Called when the game starts or when spawned
void APuzzleModel::BeginPlay()
{
	Super::BeginPlay();

	SetTickGroup(TG_PostUpdateWork);


	GetComponents(InnerMeshComponents);
	GetComponents(PuzzlePieceParentComponents);
	TotalPieces = PuzzlePieceParentComponents.Num() - 1; //Excluding the shell
	
	//InitializeAlwaysOnTopMaterials();
	SpawnBase();
	SetupModel();

	if (!PuzzleMode)
	{
		GetPuzzleGameMode();
	}

	if (PuzzleMode)
	{
		PuzzleMode->OnModelLoaded.Broadcast();
	}
	

	APuzzlePawn* PuzzlePawn = Cast<APuzzlePawn>(UGameplayStatics::GetPlayerPawn(this, 0));

	if (PuzzlePawn)
	{
		PuzzlePawn->OnPieceSelected.AddDynamic(this, &APuzzleModel::OnPieceSelected);
	}


	UE_LOG(LogTemp, Log, TEXT("BeginPlay: Loading all pieces for sculpture '%s'."), *GetActorLabel());
	LoadAllPieces();
	Explode();
	UE_LOG(LogTemp, Log, TEXT("BeginPlay: Saving initial state for sculpture '%s'."), *GetActorLabel());
	SaveAllPieces();
}

void APuzzleModel::PlayFittingSound() const
{
	UGameplayStatics::PlaySound2D(this, Fitting);
}


void APuzzleModel::SetPieceMaterial(UStaticMeshComponent* Piece, bool bAlwaysOnTop)
{

	if (bAlwaysOnTop)
	{
		Piece->SetMaterial(1, UnlitMaterial);
	}
	else {
		Piece->SetMaterial(1, LitMaterial);	
	}

	//OLD logic 
	/*
	int32 NumMaterials = Piece->GetNumMaterials();
	for (int32 Index = 0; Index < NumMaterials; ++Index)
	{
		if (bAlwaysOnTop)
		{
			// Usa o material "sempre na frente", se estiver configurado
			if (Index < PieceAlwaysOnTopMaterials.Num() && PieceAlwaysOnTopMaterials[Index])
			{
				Piece->SetMaterial(Index, PieceAlwaysOnTopMaterials[Index]);
			}
		}
		else
		{
			// Restaura o material padr�o
			if (Index < PieceDefaultMaterials.Num() && PieceDefaultMaterials[Index])
			{
				Piece->SetMaterial(Index, PieceDefaultMaterials[Index]);
			}
		}
	}*/
}



void APuzzleModel::SpawnBase()
{
	if (BaseType == EBaseType::EBase_Invalid)
	{
		UE_LOG(LogTemp, Error, TEXT("No base spawned, BaseType is Invalid!"));
		return;
	}


	if (BaseType == EBaseType::EBase_None)
	{
		UE_LOG(LogTemp, Warning, TEXT("No base spawned, BaseType is None."));
		return;
	}

	if (!PuzzleMode)
	{
		GetPuzzleGameMode();
	}


	UStaticMesh** FoundMesh = PuzzleMode->BaseMeshes.Find(BaseType);
	if (!FoundMesh || !(*FoundMesh))
	{
		UE_LOG(LogTemp, Warning, TEXT("No valid mesh found for this base type!"));
		return;
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Owner = nullptr;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	FVector SpawnLocation = GetActorLocation();
	FRotator SpawnRotation = GetActorRotation();

	AStaticMeshActor* BaseActor = GetWorld()->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), SpawnLocation, SpawnRotation, SpawnParams);
	if (BaseActor)
	{
		UStaticMeshComponent* MeshComponent = BaseActor->GetStaticMeshComponent();
		if (MeshComponent)
		{
			MeshComponent->SetStaticMesh(*FoundMesh);
		}
		SpawnedBaseActor = BaseActor;
		UE_LOG(LogTemp, Warning, TEXT("Base spawned successfully at location %s"), *SpawnLocation.ToString());
	}

}

void APuzzleModel::GetPuzzleGameMode()
{
	AGameModeBase* GameModeBase = GetWorld()->GetAuthGameMode();
	if (GameModeBase)
	{
		PuzzleMode = Cast<AGM_PuzzleMode>(GameModeBase);

		if (!PuzzleMode)
		{
			UE_LOG(LogTemp, Error, TEXT("Game mode not found!"));
		}

	}
}

// Called every frame
void APuzzleModel::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (!ScreenSidePosition || !GEngine || !GEngine->GameViewport)
		return;

	SetScreenSidePosition();
}


void APuzzleModel::SetScreenSidePosition()
{
	FVector2D ScreenPosition;
	FVector WorldLocation, WorldDirection;

	const FVector2D ViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
	const float HorizontalOffset = 150.0f;
	ScreenPosition = FVector2D(ViewportSize.X - HorizontalOffset, ViewportSize.Y * 0.5f);

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!CameraManager)
		return;

	float CurrentFOV = CameraManager->GetFOVAngle(); // Obt�m o FOV atual

	// Calcular a dist�ncia ajustada com base no FOV
	// Hipotenusa da trigonometria: dist�ncia da c�mera ao ScreenSidePosition
	float FOVRadians = FMath::DegreesToRadians(CurrentFOV * 0.5f);
	float AdjustedDistance = ViewportSize.X / (2.0f * FMath::Tan(FOVRadians)); // Rela��o entre tamanho do viewport e FOV
	AdjustedDistance *= SidePiecesDistanceFromScreen;


	// Converte a posi��o da tela para o mundo
	if (UGameplayStatics::GetPlayerController(this, 0)->DeprojectScreenPositionToWorld(
		ScreenPosition.X, ScreenPosition.Y, WorldLocation, WorldDirection))
	{
		// Calcula a nova posi��o em rela��o � c�mera
		FVector CameraLocation = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraLocation();
		FVector NewPosition = CameraLocation + (WorldDirection * AdjustedDistance); // Ajusta a dist�ncia fixa do lado direito

		// Ajusta a rota��o para acompanhar a c�mera, mas ignora o pitch (inclina��o vertical)
		FRotator CameraRotation = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraRotation();

		// Atualiza a posi��o e a rota��o
		ScreenSidePosition->SetWorldLocation(NewPosition);
		ScreenSidePosition->SetWorldRotation(CameraRotation);

		//DrawDebugSphere(GetWorld(), NewPosition, 10.f, 12, FColor::Green, false, -1.f, 0, 0.5f);

	}
}


TArray<UInnerMesh*> APuzzleModel::GetInnerMeshComponents() const
{
	return InnerMeshComponents;
}

TArray<UPuzzlePieceParentComponent*> APuzzleModel::GetPiecesToSendToBoard() const
{
	return PiecesToSendToBoard;
}

void APuzzleModel::SetupModel()
{
	if (SpawnedBaseActor)
	{
		UStaticMeshComponent* MeshComponent = SpawnedBaseActor->GetStaticMeshComponent();
		if (!MeshComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("O actor spawnado n�o possui um componente de Static Mesh!"));
			return;
		}


		FBoxSphereBounds MeshBounds = MeshComponent->CalcBounds(MeshComponent->GetComponentTransform());
		FVector TopCenter = FVector(MeshBounds.Origin.X, MeshBounds.Origin.Y, MeshBounds.Origin.Z + MeshBounds.BoxExtent.Z);
		SetActorLocation(TopCenter);


	}



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

			if ( (!loadedFromSaveGame && skippedPieces < InitialPieces) || (PuzzlePiece->GetIsLocked()) )
			{
				skippedPieces++;
				PuzzlePiece->SetIsLocked(true);
				PuzzlePiece->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
				PuzzlePiece->SetIsOnBoard(false);
				PuzzlePiece->SetWorldLocation(PuzzlePiece->GetParentInitialWorldPosition());
				PuzzlePiece->SetWorldRotation(PuzzlePiece->GetParentInitialWorldRotator());
				CorrectPiecesPlaced++;
				SetPieceMaterial(PuzzlePiece->GetPieceMesh(), false);
			}
			else {
				PuzzlePiece->SetIsLocked(false);
				PuzzlePiece->SetIsOnBoard(true);
				PiecesToSendToBoard.Add(PuzzlePiece);
				SetPieceMaterial(PuzzlePiece->GetPieceMesh(), true);
			}


		}
	}

	PiecesInBoard = PiecesToSendToBoard;

	float PiecesByCircle = 360 / DegreeSpaceBetweenPieces;
	int CircleCount = UKismetMathLibrary::FCeil(PiecesToSendToBoard.Num() / PiecesByCircle);
	ShuffleArray(PiecesToSendToBoard);

	//MovePiecesToCylinder();

	MovePiecesToScreenSide(true);


	if (PuzzleMode)
	{
		PuzzleMode->OnModelExploded.Broadcast();
	}
}


void APuzzleModel::MovePiecesToScreenSide(bool firstTime = false)
{

	const float ColumnOffset = 3.0f; // Dist�ncia lateral entre as colunas
	const float RowOffset = 5.0f;    // Espa�amento vertical entre as pe�as
	
	const FVector BasePosition = ScreenSidePosition->GetComponentLocation();

	const int32 TotalPiecesOnBoard = PiecesInBoard.Num();

	int rowDirection = 1;

	int32 CurrentIndex = 0;
	FVector CameraLocation = UGameplayStatics::GetPlayerCameraManager(this, 0)->GetCameraLocation();

	for (UPuzzlePieceParentComponent* PieceParent : PiecesInBoard)
	{
		int32 CurrentRow = CurrentIndex / 2;
		int32 CurrentColumn = CurrentIndex % 2;

		float ColumnDirection = (CurrentColumn == 0) ? -1.0f : 1.0f; // Esquerda (-1) ou direita (+1)
		float XOffset = ColumnDirection * ColumnOffset;
		float ZOffset = CurrentRow * RowOffset * rowDirection;

		if (CurrentColumn == 0)
		{
			rowDirection = -rowDirection;
		}

		// Calculates the new position relative to the ScreenSidePosition
		FVector LocalOffset = FVector(0.0f, XOffset, ZOffset); // Y controla as colunas, Z as linhas
		FVector NewPosition = ScreenSidePosition->GetComponentTransform().TransformPosition(LocalOffset);

		// Updates the piece parent
		PieceParent->AttachToComponent(ScreenSidePosition, FAttachmentTransformRules::KeepWorldTransform);
		PieceParent->SetWorldLocation(NewPosition);

		if (firstTime)
		{
			FRotator LookAtRotation = UKismetMathLibrary::FindLookAtRotation(NewPosition, CameraLocation);

			FRotator BaseRotation = FRotator(90.0f, 90.0f, 90.0f);

			if (CurrentColumn == 0)
			{
				BaseRotation += FRotator(0.0f, 180.0f, 0.0f);
			}

			FRotator AdjustedRotation = LookAtRotation + BaseRotation;

			PieceParent->SetWorldRotation(AdjustedRotation);
		}
		else {

			FRotator AdjustedRotation = FRotator(0.0f, 0.0f, 90.0f);
			PieceParent->SetRelativeRotation(AdjustedRotation);
	
		}
		
		PieceParent->SetWorldScale3D(PieceParent->GetComponentScale() * PiecesScaleFactor);

		if (CurrentIndex == 0)
		{
			UE_LOG(LogTemp, Log, TEXT("Piece Location: %s"), *PieceParent->GetComponentLocation().ToString());
			UE_LOG(LogTemp, Log, TEXT("Piece Rotation: %s"), *PieceParent->GetComponentRotation().ToString());
		}

		CurrentIndex++;

		

	}

}



void APuzzleModel::MovePiecesToCylinder()
{
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
}

void APuzzleModel::ResetModel()
{
	UPuzzleSaveGame::ClearSaveGame(GetActorLabel(), 0);

	for (UPuzzlePieceParentComponent* PuzzlePiece : PuzzlePieceParentComponents)
	{
		if (PuzzlePiece != nullptr)
		{

			if (PuzzlePiece->GetIsShell())
			{
				continue;
			}

			PuzzlePiece->SetCanLockPieces(CanLockPieces);
			PuzzlePiece->SetIsLocked(false);
		}
	}
	loadedFromSaveGame = false;
	Explode();
	SaveAllPieces();
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

void APuzzleModel::OnPieceSelected(UPuzzlePieceParentComponent* piece)
{
	if (PiecesInBoard.Contains(piece))
	{
		piece->AttachToComponent(RootComponent, FAttachmentTransformRules::KeepWorldTransform);
		piece->SetWorldScale3D(FVector::One());
	}
}

void APuzzleModel::OnPiecePlaced(UPuzzlePieceParentComponent* piece)
{
	CorrectPiecesPlaced++;

	OnPiecePlacedEvent.Broadcast();
	PiecesInBoard.Remove(piece);
	SavePiece(piece->GetIdentifier(), true);
	SetPieceMaterial(piece->GetPieceMesh(), false);

	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,                      // Unique message key
			5.0f,                    // Duration in seconds
			FColor::Green,            // Text color
			FString::Printf(TEXT("Placed %s"), *piece->GetIdentifier())  // Corrected format
		);
	}

}

void APuzzleModel::OnPieceDropped(UPuzzlePieceParentComponent* piece)
{
	if (GEngine)
	{
		GEngine->AddOnScreenDebugMessage(
			-1,                      // Unique message key
			5.0f,                    // Duration in seconds
			FColor::Yellow,            // Text color
			FString::Printf(TEXT("Dropped %s"), *piece->GetIdentifier())  // Corrected format
		);
	}


	MovePiecesToScreenSide();
}

void APuzzleModel::SetInitialPieces(int32 pieces)
{
	InitialPieces = pieces;
}


