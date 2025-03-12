// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "GM_PuzzleMode.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnModelLoadedEnded);
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnModelExplodedEnded);



UENUM(BlueprintType)
enum class EBaseType : uint8
{
	EBase_Invalid   UMETA(DisplayName = "Invalid"),
	EBase_None		UMETA(DisplayName = "None"),
	EBase_Basic		UMETA(DisplayName = "Basic"),
	EBase_Classic	UMETA(DisplayName = "Classic"),
};


/**
 * 
 */
UCLASS()
class PUZZLE3D_API AGM_PuzzleMode : public AGameModeBase
{
	GENERATED_BODY()

public:

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnModelLoadedEnded OnModelLoaded;

	UPROPERTY(BlueprintAssignable, Category = "Events")
	FOnModelExplodedEnded OnModelExploded;

	UPROPERTY(EditAnywhere, Category = "Mesh Mapping")
	TMap<EBaseType, UStaticMesh*> BaseMeshes;

	
};
