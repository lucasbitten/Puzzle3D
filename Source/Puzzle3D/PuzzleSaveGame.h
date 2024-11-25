// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PuzzlePieceParentComponent.h"
#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "PuzzleSaveGame.generated.h"

USTRUCT(BlueprintType)
struct FPuzzlePieceSaveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString PieceID;

    UPROPERTY(BlueprintReadWrite)
    bool IsCorrectlyPositioned;
};

USTRUCT(BlueprintType)
struct FPuzzleSaveData
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadWrite)
    FString SculptureName;

    UPROPERTY(BlueprintReadWrite)
    TArray<FPuzzlePieceSaveData> Pieces;
};

/**
 * 
 */
UCLASS()
class PUZZLE3D_API UPuzzleSaveGame : public USaveGame
{
	GENERATED_BODY()
public:
    UPROPERTY(BlueprintReadWrite)
    TArray<FPuzzleSaveData> SavedPuzzles;


    UPuzzleSaveGame* LoadOrCreateSaveGame();


    // Fun��o para salvar todas as pe�as de uma escultura
    void SaveAllPiecesState(FString SculptureName, const TArray<UPuzzlePieceParentComponent*>& Pieces);

    // Fun��o para salvar ou modificar o estado de uma pe�a espec�fica
    void SavePieceState(FString SculptureName, FString PieceID, bool IsCorrectlyPositioned);

    // Fun��o para carregar o estado de todas as pe�as de uma escultura
    TArray<FPuzzlePieceSaveData> LoadAllPiecesState(FString SculptureName);

    // Fun��o para carregar o estado de uma pe�a espec�fica
    bool LoadPieceState(FString SculptureName, FString PieceID, bool& IsCorrectlyPositioned);

    // Fun��o para limpar o save game
    static bool ClearSaveGame(FString SlotName, int32 UserIndex = 0);
};