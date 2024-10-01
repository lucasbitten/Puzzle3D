// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "DatasmithSceneActor.h"
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

	UFUNCTION(BlueprintCallable, CallInEditor)
	void GenerateModelBlueprint();

	void HarvestComponentsAndCreateBlueprint(ADatasmithSceneActor* SceneActor, UClass* ParentClass, UCurveFloat* AlphaCurve);

	UFUNCTION(CallInEditor, Category = "Scene Setup")
	void SetAllStaticMeshesHiddenInSceneCapture();

	UCurveFloat* LoadCurveFromPath(const FString& CurvePath);

};
