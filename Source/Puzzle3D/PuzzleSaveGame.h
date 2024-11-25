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


    // Função para salvar todas as peças de uma escultura
    void SaveAllPiecesState(FString SculptureName, const TArray<UPuzzlePieceParentComponent*>& Pieces);

    // Função para salvar ou modificar o estado de uma peça específica
    void SavePieceState(FString SculptureName, FString PieceID, bool IsCorrectlyPositioned);

    // Função para carregar o estado de todas as peças de uma escultura
    TArray<FPuzzlePieceSaveData> LoadAllPiecesState(FString SculptureName);

    // Função para carregar o estado de uma peça específica
    bool LoadPieceState(FString SculptureName, FString PieceID, bool& IsCorrectlyPositioned);

    // Função para limpar o save game
    static bool ClearSaveGame(FString SlotName, int32 UserIndex = 0);
};