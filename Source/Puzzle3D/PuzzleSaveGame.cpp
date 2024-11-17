#include "PuzzleSaveGame.h"
#include "Kismet/GameplayStatics.h"

// Função para salvar todas as peças de uma escultura
void UPuzzleSaveGame::SaveAllPiecesState(FString SculptureName, const TArray<UPuzzlePieceParentComponent*>& Pieces)
{
    // Encontre ou crie o dado da escultura
    FPuzzleSaveData* PuzzleData = SavedPuzzles.FindByPredicate([&](const FPuzzleSaveData& Data) {
        return Data.SculptureName == SculptureName;
        });

    if (!PuzzleData)
    {
        PuzzleData = &SavedPuzzles.AddDefaulted_GetRef();
        PuzzleData->SculptureName = SculptureName;
    }

    // Limpar as peças antigas e adicionar as novas
    PuzzleData->Pieces.Empty();

    for (UPuzzlePieceParentComponent* Piece : Pieces)
    {
        FPuzzlePieceSaveData PieceData;
        PieceData.PieceID = Piece->GetIdentifier();
        PieceData.IsCorrectlyPositioned = Piece->GetIsLocked();

        PuzzleData->Pieces.Add(PieceData);
    }

    // Salve o progresso
    bool bSaveSuccessful = UGameplayStatics::SaveGameToSlot(this, TEXT("SaveSlot1"), 0);
    if (bSaveSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Save successful!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Save failed!"));
    }

    // Exibir os dados salvos
    UE_LOG(LogTemp, Log, TEXT("Sculpture: %s"), *SculptureName);
    for (const FPuzzlePieceSaveData& Piece : PuzzleData->Pieces)
    {
        UE_LOG(LogTemp, Log, TEXT("Piece: %s, Correctly Positioned: %d"), *Piece.PieceID, Piece.IsCorrectlyPositioned);
    }
}

// Função para salvar ou modificar o estado de uma peça específica
void UPuzzleSaveGame::SavePieceState(FString SculptureName, FString PieceID, bool IsCorrectlyPositioned)
{
    FPuzzleSaveData* PuzzleData = SavedPuzzles.FindByPredicate([&](const FPuzzleSaveData& Data) {
        return Data.SculptureName == SculptureName;
        });

    if (!PuzzleData)
    {
        PuzzleData = &SavedPuzzles.AddDefaulted_GetRef();
        PuzzleData->SculptureName = SculptureName;
    }

    // Procura a peça pelo Identifier
    FPuzzlePieceSaveData* PieceData = PuzzleData->Pieces.FindByPredicate([&](const FPuzzlePieceSaveData& Data) {
        return Data.PieceID == PieceID;
        });

    if (!PieceData)
    {
        PieceData = &PuzzleData->Pieces.AddDefaulted_GetRef();
        PieceData->PieceID = PieceID;
    }

    PieceData->IsCorrectlyPositioned = IsCorrectlyPositioned;

    bool bSaveSuccessful = UGameplayStatics::SaveGameToSlot(this, TEXT("SaveSlot1"), 0);
    if (bSaveSuccessful)
    {
        UE_LOG(LogTemp, Log, TEXT("Piece save successful!"));
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Piece save failed!"));
    }

    // Exibir os dados da peça salva
    UE_LOG(LogTemp, Log, TEXT("Sculpture: %s, Piece: %s, Correctly Positioned: %d"),
        *SculptureName, *PieceID, IsCorrectlyPositioned);
}

// Função para carregar o estado de todas as peças de uma escultura
TArray<FPuzzlePieceSaveData> UPuzzleSaveGame::LoadAllPiecesState(FString SculptureName)
{
    TArray<FPuzzlePieceSaveData> LoadedPieces;

    // Carregar o save game
    UPuzzleSaveGame* SaveGameInstance = Cast<UPuzzleSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("SaveSlot1"), 0));
    if (SaveGameInstance)
    {
        FPuzzleSaveData* PuzzleData = SaveGameInstance->SavedPuzzles.FindByPredicate([&](const FPuzzleSaveData& Data) {
            return Data.SculptureName == SculptureName;
            });

        if (PuzzleData)
        {
            LoadedPieces = PuzzleData->Pieces;
            UE_LOG(LogTemp, Log, TEXT("All pieces loaded successfully!"));

            // Exibir os dados das peças carregadas
            for (const FPuzzlePieceSaveData& Piece : LoadedPieces)
            {
                UE_LOG(LogTemp, Log, TEXT("Piece: %s, Correctly Positioned: %d"), *Piece.PieceID, Piece.IsCorrectlyPositioned);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Sculpture data not found in save file!"));
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Load failed!"));
    }

    return LoadedPieces;
}

// Função para carregar o estado de uma peça específica
bool UPuzzleSaveGame::LoadPieceState(FString SculptureName, FString PieceID, bool& IsCorrectlyPositioned)
{
    // Carregar o save game
    UPuzzleSaveGame* SaveGameInstance = Cast<UPuzzleSaveGame>(UGameplayStatics::LoadGameFromSlot(TEXT("SaveSlot1"), 0));
    if (SaveGameInstance)
    {
        FPuzzleSaveData* PuzzleData = SaveGameInstance->SavedPuzzles.FindByPredicate([&](const FPuzzleSaveData& Data) {
            return Data.SculptureName == SculptureName;
            });

        if (PuzzleData)
        {
            // Procura a peça pelo Identifier
            FPuzzlePieceSaveData* PieceData = PuzzleData->Pieces.FindByPredicate([&](const FPuzzlePieceSaveData& Data) {
                return Data.PieceID == PieceID;
                });

            if (PieceData)
            {
                IsCorrectlyPositioned = PieceData->IsCorrectlyPositioned;
                UE_LOG(LogTemp, Log, TEXT("Piece state loaded successfully! Sculpture: %s, Piece: %s, Correctly Positioned: %d"),
                    *SculptureName, *PieceID, PieceData->IsCorrectlyPositioned);
                return true;
            }
            else
            {
                UE_LOG(LogTemp, Error, TEXT("Piece with ID %s not found in the save data for Sculpture: %s!"),
                    *PieceID, *SculptureName);
            }
        }
        else
        {
            UE_LOG(LogTemp, Error, TEXT("Sculpture data for %s not found in the save file!"), *SculptureName);
        }
    }
    else
    {
        UE_LOG(LogTemp, Error, TEXT("Failed to load the save game from the slot!"));
    }

    return false;
}
