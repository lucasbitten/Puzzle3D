// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/StaticMeshComponent.h"
#include "PuzzlePiecesComponent.generated.h"

/**
 * 
 */
UCLASS()
class PUZZLE3D_API UPuzzlePiecesComponent : public UStaticMeshComponent
{
	GENERATED_BODY()
	
protected:
	virtual void BeginPlay() override;

};
