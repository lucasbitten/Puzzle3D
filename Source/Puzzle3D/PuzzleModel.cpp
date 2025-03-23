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

	BoardMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BoardMesh"));
	BoardMesh->AttachToComponent(ScreenSidePosition, FAttachmentTransformRules::SnapToTargetNotIncludingScale);

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

// Função para carregar o estado de uma peça específica
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
		PuzzlePawn->CurrentPuzzleModel = this;
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
		if (PieceAlwaysOnTopMaterials.Num() == 2 && PieceAlwaysOnTopMaterials[1])
		{
			Piece->SetMaterial(1, PieceAlwaysOnTopMaterials[1]);
		}
	}
	else {
		if (PieceAlwaysOnTopMaterials.Num() == 2 && PieceDefaultMaterials[1])
		{
			Piece->SetMaterial(1, PieceDefaultMaterials[1]);
		}
	}

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
	FVector WorldTopLimit, WorldBottomLimit, TopDirection, BottomDirection;

	// Retrieve the viewport size
	const FVector2D ViewportSize = GEngine->GameViewport->Viewport->GetSizeXY();
	const float HorizontalOffset = 150.0f;
	ScreenPosition = FVector2D(ViewportSize.X - HorizontalOffset, ViewportSize.Y * 0.5f);

	APlayerController* PlayerController = UGameplayStatics::GetPlayerController(this, 0);

	APlayerCameraManager* CameraManager = UGameplayStatics::GetPlayerCameraManager(this, 0);
	if (!CameraManager)
		return;

	// Get the current field of view
	float CurrentFOV = CameraManager->GetFOVAngle();
	float FOVRadians = FMath::DegreesToRadians(CurrentFOV * 0.5f);
	float AdjustedDistance = ViewportSize.X / (2.0f * FMath::Tan(FOVRadians));
	AdjustedDistance *= SidePiecesDistanceFromScreen;

	// Convert screen position to world position
	if (PlayerController->DeprojectScreenPositionToWorld(ScreenPosition.X, ScreenPosition.Y, WorldLocation, WorldDirection) &&
		PlayerController->DeprojectScreenPositionToWorld(ScreenPosition.X, 0.0f, WorldTopLimit, TopDirection) &&
		PlayerController->DeprojectScreenPositionToWorld(ScreenPosition.X, ViewportSize.Y, WorldBottomLimit, BottomDirection))
	{
		// Compute the camera location
		FVector CameraLocation = CameraManager->GetCameraLocation();
		FVector NewPosition = CameraLocation + (WorldDirection * AdjustedDistance);

		//For bottom and top max/min
		FVector TopPosition = CameraLocation + (TopDirection * AdjustedDistance);
		FVector BottomPosition = CameraLocation + (BottomDirection * AdjustedDistance);

		float ScrollFactor = boardScrollAmount / 100.0f;

		// Get the start and end world positions of the board (calculated from the relative positions of pieces on the board)
		FVector WorldStartPos = ScreenSidePosition->GetComponentTransform().TransformPosition(PiecesInBoard[0]->GetBoardPosition()); // Top position
		FVector WorldEndPos = ScreenSidePosition->GetComponentTransform().TransformPosition(PiecesInBoard.Last()->GetBoardPosition()); // Bottom position

		float OffSet = 12.5f;

		// Get the local height of the plane
		float PlaneHeight = FMath::Abs((WorldStartPos.Z-OffSet) - (WorldEndPos.Z+OffSet)) / 2.0f;
		// We want the **top** of the plane to match `WorldTopLimit` when CustomPoint = 0
		// and the **bottom** of the plane to match `WorldBottomLimit` when CustomPoint = 100
		float WorldZ = FMath::Lerp(TopPosition.Z - PlaneHeight, BottomPosition.Z + PlaneHeight, ScrollFactor);

		// Set the new Z position
		//NewPosition.Z = WorldZ;
		ScreenSidePosition->SetWorldLocation(NewPosition);

		//ScreenSidePosition->SetWorldLocation(NewPosition);

		FVector relativePosition = BoardMesh->GetRelativeLocation();
		relativePosition.Z = WorldZ;
		BoardMesh->SetRelativeLocation(relativePosition);
		/*
		DrawDebugSphere(GetWorld(), WorldStartPos, 10.f, 12, FColor::Green, false, -1.f, 0, 0.5f);
		DrawDebugSphere(GetWorld(), WorldEndPos, 10.f, 12, FColor::Green, false, -1.f, 0, 0.5f);
		DrawDebugSphere(GetWorld(), NewPosition, 10.f, 12, FColor::Magenta, false, -1.f, 0, 0.5f);
		DrawDebugSphere(GetWorld(), TopPosition, 10.f, 12, FColor::Blue, false, -1.f, 0, 0.5f);
		DrawDebugSphere(GetWorld(), BottomPosition, 10.f, 12, FColor::Blue, false, -1.f, 0, 0.5f);
		*/
		FRotator CameraRotation = CameraManager->GetCameraRotation();

		// Apply the final world position and rotation
		ScreenSidePosition->SetWorldRotation(CameraRotation);
	}
}

void APuzzleModel::SetBoardScrollAmount(float mouseDeltaY)
{
	// Convert MouseDeltaY into `boardScrollAmount` change
	boardScrollAmount += ( -mouseDeltaY * boardScrollSpeed);

	// Clamp customPos to be within bounds (0 to 100)
	boardScrollAmount = FMath::Clamp(boardScrollAmount, 0.0f, 100.0f);

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
			UE_LOG(LogTemp, Error, TEXT("O actor spawnado não possui um componente de Static Mesh!"));
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
			PuzzlePiece->SetParentInitialWorldScale(PuzzlePiece->GetComponentScale());

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
	PiecesInBoard.Empty();

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
				PuzzlePiece->SetWorldScale3D(PuzzlePiece->GetParentInitialWorldScale());
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

	//float PiecesByCircle = 360 / DegreeSpaceBetweenPieces;
	//int CircleCount = UKismetMathLibrary::FCeil(PiecesToSendToBoard.Num() / PiecesByCircle);
	
	ShuffleArray(PiecesToSendToBoard);

	//MovePiecesToCylinder();
	
	MovePiecesToScreenSide();

	if (PuzzleMode)
	{
		PuzzleMode->OnModelExploded.Broadcast();
	}
}


void APuzzleModel::MovePiecesToScreenSide()
{

	const int32 TotalPiecesOnBoard = PiecesInBoard.Num();
	int32 CurrentIndex = 0;

	// Adjust number of columns dynamically based on total pieces
	const int32 NumColumns = boardColumns;
	const int32 NumRows = FMath::CeilToInt((float)TotalPiecesOnBoard / NumColumns);

	// 4️⃣ Adjust spacing based on scale to prevent overlapping
	const float ColumnOffset = boardColumnOffset * boardPieceScaleFactor; // Increase spacing if scale is bigger
	const float RowOffset = boardRowOffset * boardPieceScaleFactor;

	int rowDirection = 1;

	for (UPuzzlePieceParentComponent* PieceParent : PiecesInBoard)
	{

		// Updates the piece parent
		//PieceParent->AttachToComponent(ScreenSidePosition, FAttachmentTransformRules::KeepWorldTransform);
		PieceParent->AttachToComponent(BoardMesh, FAttachmentTransformRules::KeepWorldTransform);
		int32 CurrentRow = CurrentIndex / NumColumns;
		int32 CurrentColumn = CurrentIndex % NumColumns;
		
		float ColumnDirection = (CurrentColumn == 0) ? -1.0f : 1.0f; // Left (-1) or Right (+1)

		// Adjust for centering
		float XOffset = (CurrentColumn - (NumColumns - 1) * 0.5f) * ColumnOffset;
		float ZOffset = (CurrentRow - (NumRows - 1) * 0.5f) * RowOffset;


		if (CurrentColumn == 0)
		{
			rowDirection = -rowDirection;
		}

		// Calculates the new position relative to the ScreenSidePosition
		FVector LocalOffset = FVector(0.0f, XOffset, ZOffset); // Y -> Columns, Z -> Rows

		
		FVector NewWorldPosition = ScreenSidePosition->GetComponentTransform().TransformPosition(LocalOffset);
		FVector ConvertedRelativePosition = ScreenSidePosition->GetComponentTransform().InverseTransformPosition(NewWorldPosition);
		ConvertedRelativePosition += FVector(0.0f, boardGridOffset.X, 0.0f);

		FVector NewPosition = ScreenSidePosition->GetComponentTransform().TransformPosition(ConvertedRelativePosition);
		
		//Location (World and Relative to parent)
		PieceParent->SetWorldLocation(NewPosition);
		FVector RelativePosition = ScreenSidePosition->GetComponentTransform().InverseTransformPosition(PieceParent->GetComponentLocation());
		
		//Rotation of the pieces relative to the ScreenSidePosition transform.
		FRotator AdjustedRotation = FRotator(0.0f, 0.0f, 90.0f);
		PieceParent->SetRelativeRotation(AdjustedRotation);

		//Scale of the pieces
		PieceParent->SetWorldScale3D(FVector(boardPieceScaleFactor));
		FVector NewScale = PieceParent->GetComponentScale() * PiecesScaleFactor;
		PieceParent->SetWorldScale3D(NewScale);

		//Board properties for future resets
		PieceParent->SetBoardProperties(BoardMesh, ScreenSidePosition, RelativePosition, AdjustedRotation, PieceParent->GetComponentRotation(), NewScale);

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


	MovePiecesToScreenSide();

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

}

void APuzzleModel::SetInitialPieces(int32 pieces)
{
	InitialPieces = pieces;
}


