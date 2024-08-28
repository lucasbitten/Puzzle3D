// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "AssetActionUtility.h"
#include "AAU_UpdateModelMeshes.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLE3DEDITOR_API UAAU_UpdateModelMeshes : public UAssetActionUtility
{
	GENERATED_BODY()
public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "MyUtilities")
	void AddPiecesComponentToMeshes();

	UFUNCTION(BlueprintCallable, CallInEditor, Category = "MyUtilities")
	void RemoveReorientedPlanes();  
};
