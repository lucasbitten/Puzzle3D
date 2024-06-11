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

public:

	void SetInitialRelativePosition(FVector initialPos);

	UFUNCTION(BlueprintCallable)
	const FVector GetInitialRelativePosition() const;

	void SetInitialNormal(FVector direction);

	UFUNCTION(BlueprintCallable)
	const FVector GetInitialNormal() const;

	void SetShellRelativePosition(FVector position);

	UFUNCTION(BlueprintCallable)
	const FVector GetShellRelativePosition() const;


	UFUNCTION(BlueprintCallable)
	const bool GetIsLocked() const;

	UFUNCTION(BlueprintCallable)
	const void SetIsLocked(bool locked);

	const bool GetIsShell() const;

	const void SetIsShell(bool isShell);

private:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FVector InitialRelativePosition;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FVector InitialNormal;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	FVector ShellRelativePosition;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	bool IsLocked;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Initial Info", meta = (AllowPrivateAccess = "true"))
	bool IsShell;

	UFUNCTION(BlueprintCallable)
	const void AlignToSurfaceNormal(FVector currentNormal);



};
