#include "PuzzleSaveGame.h"
#include "Kismet/GameplayStatics.h"


UPuzzleSaveGame* UPuzzleSaveGame::LoadOrCreateSaveGame()
{
    UPuzzleSaveGame* SaveGameInstance = Cast<UPuzzleSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("SaveSlot1"), 0));
    if (!SaveGameInstance)
    {
        SaveGameInstance = Cast<UPuzzleSaveGame>(UGameplayStatics::CreateSaveGameObject(UPuzzleSaveGame::StaticClass()));
        UE_LOG(LogTemp, Warning, TEXT("No save game found. A new save game instance was created."));
    }
    return SaveGameInstance;
}


void UPuzzleSaveGame::SaveAllPiecesState(FString SculptureName, const TArray<UPuzzlePieceParentComponent*>& Pieces)
{
    UPuzzleSaveGame* SaveGameInstance = LoadOrCreateSaveGame();


    FPuzzleSaveData* PuzzleData = SaveGameInstance->SavedPuzzles.FindByPredicate([&](const FPuzzleSaveData& Data) {
        return Data.SculptureName == SculptureName;
        });

    if (!PuzzleData)
    {
        PuzzleData = &SaveGameInstance->SavedPuzzles.AddDefaulted_GetRef();
        PuzzleData->SculptureName = SculptureName;
    }

    PuzzleData->Pieces.Empty();

    for (UPuzzlePieceParentComponent* Piece : Pieces)
    {
        if (!Piece)
        {
            continue;
        }

        FPuzzlePieceSaveData PieceData;
        PieceData.PieceID = Piece->GetIdentifier();
        PieceData.IsCorrectlyPositioned = Piece->GetIsLocked();

        PuzzleData->Pieces.Add(PieceData);

        if (PieceData.IsCorrectlyPositioned)
        {
            UE_LOG(LogTemp, Log, TEXT("Saved Piece: %s, Correctly Positioned: %d"), *PieceData.PieceID, PieceData.IsCorrectlyPositioned);
        }

    }

    // Salve o progresso
    bool bSaveSuccessful = UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("SaveSlot1"), 0);
    if (bSaveSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Save successful!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Save failed!"));
    }

    // Exibir os dados salvos
    //UE_LOG(LogTemp, Log, TEXT("Sculpture: %s"), *SculptureName);
    //for (const FPuzzlePieceSaveData& Piece : PuzzleData->Pieces)
    //{
    //    UE_LOG(LogTemp, Log, TEXT("Piece: %s, Correctly Positioned: %d"), *Piece.PieceID, Piece.IsCorrectlyPositioned);
    //}
}

void UPuzzleSaveGame::SavePieceState(FString SculptureName, FString PieceID, bool IsCorrectlyPositioned)
{
    // Carregar o save game existente
    UPuzzleSaveGame* SaveGameInstance = LoadOrCreateSaveGame();

    // Atualizar ou adicionar os dados da escultura
    FPuzzleSaveData* PuzzleData = SaveGameInstance->SavedPuzzles.FindByPredicate([&](const FPuzzleSaveData& Data) {
        return Data.SculptureName == SculptureName;
        });

    if (!PuzzleData)
    {
        PuzzleData = &SaveGameInstance->SavedPuzzles.AddDefaulted_GetRef();
        PuzzleData->SculptureName = SculptureName;
    }

    // Atualizar ou adicionar os dados da peça
    FPuzzlePieceSaveData* PieceData = PuzzleData->Pieces.FindByPredicate([&](const FPuzzlePieceSaveData& Data) {
        return Data.PieceID == PieceID;
        });

    if (!PieceData)
    {
        PieceData = &PuzzleData->Pieces.AddDefaulted_GetRef();
        PieceData->PieceID = PieceID;
    }

    PieceData->IsCorrectlyPositioned = IsCorrectlyPositioned;

    // Salvar o progresso
    bool bSaveSuccessful = UGameplayStatics::SaveGameToSlot(SaveGameInstance, TEXT("SaveSlot1"), 0);
    if (bSaveSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Piece save successful!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Piece save failed!"));
    }
}

TArray<FPuzzlePieceSaveData> UPuzzleSaveGame::LoadAllPiecesState(FString SculptureName)
{
    TArray<FPuzzlePieceSaveData> LoadedPieces;

    UPuzzleSaveGame* SaveGameInstance = LoadOrCreateSaveGame();

    FPuzzleSaveData* PuzzleData = SaveGameInstance->SavedPuzzles.FindByPredicate([&](const FPuzzleSaveData& Data) {
        return Data.SculptureName == SculptureName;
        });

    if (PuzzleData)
    {
        LoadedPieces = PuzzleData->Pieces;
        UE_LOG(LogTemp, Log, TEXT("All pieces loaded successfully for Sculpture: %s"), *SculptureName);

        // Exibir os dados das peças carregadas
        for (const FPuzzlePieceSaveData& Piece : LoadedPieces)
        {
            if (Piece.IsCorrectlyPositioned)
            {
                UE_LOG(LogTemp, Log, TEXT("Loaded Piece: %s, Correctly Positioned: %d"), *Piece.PieceID, Piece.IsCorrectlyPositioned);
            }
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No data found for Sculpture: %s"), *SculptureName);
    }

    return LoadedPieces;
}

// Função para carregar o estado de uma peça específica
bool UPuzzleSaveGame::LoadPieceState(FString SculptureName, FString PieceID, bool& IsCorrectlyPositioned)
{
    // Inicializar explicitamente para evitar problemas
    IsCorrectlyPositioned = false;

    // Carregar o save game
    UPuzzleSaveGame* SaveGameInstance = LoadOrCreateSaveGame();

    FPuzzleSaveData* PuzzleData = SaveGameInstance->SavedPuzzles.FindByPredicate([&](const FPuzzleSaveData& Data) {
        return Data.SculptureName == SculptureName;
        });

    if (!PuzzleData)
    {
        UE_LOG(LogTemp, Warning, TEXT("No data found for Sculpture: %s"), *SculptureName);
        return false;
    }

    FPuzzlePieceSaveData* PieceData = PuzzleData->Pieces.FindByPredicate([&](const FPuzzlePieceSaveData& Data) {
        return Data.PieceID == PieceID;
        });

    if (!PieceData)
    {
        UE_LOG(LogTemp, Warning, TEXT("Piece with ID %s not found for Sculpture: %s"), *PieceID, *SculptureName);
        return false;
    }

    IsCorrectlyPositioned = PieceData->IsCorrectlyPositioned;
    UE_LOG(LogTemp, Log, TEXT("Piece state loaded successfully! Sculpture: %s, Piece: %s, Correctly Positioned: %d"),
        *SculptureName, *PieceID, PieceData->IsCorrectlyPositioned);

    return true;
}

bool UPuzzleSaveGame::ClearSaveGame(FString SlotName, int32 UserIndex)
{
    if (UGameplayStatics::DoesSaveGameExist(SlotName, UserIndex))
    {
        bool bDeleted = UGameplayStatics::DeleteGameInSlot(SlotName, UserIndex);
        if (bDeleted)
        {
            UE_LOG(LogTemp, Log, TEXT("Save game in slot '%s' cleared successfully."), *SlotName);
            return true;
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("Failed to clear save game in slot '%s'."), *SlotName);
            return false;
        }
    }
    else
    {
        UE_LOG(LogTemp, Warning, TEXT("No save game found in slot '%s' to clear."), *SlotName);
        return false;
    }
}