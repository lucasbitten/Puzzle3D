// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "PuzzleModel.h"
#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "GI_Puzzle.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLE3D_API UGI_Puzzle : public UGameInstance
{
	GENERATED_BODY()
	
public:

	UPROPERTY(BlueprintReadWrite)
	TSubclassOf<APuzzleModel> ModelToSpawn;

	UPROPERTY(BlueprintReadWrite)
	bool ShowHints = true;

	UPROPERTY(BlueprintReadWrite)
	bool InvertX = true;

	UPROPERTY(BlueprintReadWrite)
	bool InvertY;

};
