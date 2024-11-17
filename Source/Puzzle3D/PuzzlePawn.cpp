// Fill out your copyright notice in the Description page of Project Settings.

#include "PuzzlePawn.h"
#include "Camera/CameraComponent.h"

// Sets default values
APuzzlePawn::APuzzlePawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void APuzzlePawn::BeginPlay()
{
	Super::BeginPlay();
	SetGrabMode(true);

	auto GameInstance = GetGameInstance();
	if (UGI_Puzzle* GI_Puzzle = Cast<UGI_Puzzle>(GameInstance))
	{
		PuzzleGameInstance = GI_Puzzle;
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("Failed to cast Game Instance to  GI_Puzzle"));
	}

}

void APuzzlePawn::OnLeftMouseButtonPressed()
{
	if (CurrentPieceComponent)
	{
		return;
	}

	FVector RayStart, RayEnd;


	if (CreateRayFromMouseLocation(RayStart, RayEnd))
	{
		HandleOnWorldPress();	
	}

}

void APuzzlePawn::OnLeftMouseButtonReleased()
{
	if (!CanMovePiece)
	{
		return;
	}

	if (GrabMode && CurrentPieceComponent)
	{
		if (CalculateDistanceFromCurrentPosition() < CorrectPositionTolerance)
		{
			CurrentPieceComponent->StartLerpingToCorrectPositionWithOffset();
			CanMovePiece = false;
			CurrentPieceComponent->OnLerpToCorrectPositionCompletedCallback.AddDynamic(this, &APuzzlePawn::OnLerpToCorrectPositionCompletedCallback);

			CurrentPieceComponent->SetIsLocked(true);

		}
		else
		{
			DeselectPieceComponent();
			CurrentPieceComponent->StopLerpCloseToCameraTimeline();
			CurrentPieceComponent = nullptr;
			CanMovePiece = true;
		}
	}
}

void APuzzlePawn::OnRightMouseButtonPressed()
{
	if (!CanMovePiece)
	{
		return;
	}

	SetGrabMode(false);
}

void APuzzlePawn::OnRightMouseButtonReleased()
{
	if (!CanMovePiece)
	{
		return;
	}

	SetGrabMode(true);
}


void APuzzlePawn::OnLerpToCorrectPositionCompletedCallback()
{
	DeselectPieceComponent();
	if (CurrentPieceComponent)
	{
		CurrentPieceComponent->OnLerpToCorrectPositionCompletedCallback.RemoveDynamic(this, &APuzzlePawn::OnLerpToCorrectPositionCompletedCallback);
	}
	CanMovePiece = true;
	CurrentPieceComponent = nullptr;
	SetGrabMode(true);

}

void APuzzlePawn::OnPieceLerpCloseToCameraCompletedCallback()
{
	CanMovePiece = true;

	if (CurrentPieceComponent)
	{
		CurrentPieceComponent->OnLerpToCameraCompletedCallback.RemoveDynamic(this, &APuzzlePawn::OnPieceLerpCloseToCameraCompletedCallback);
	}
}



bool APuzzlePawn::CreateRayFromMouseLocation(FVector& RayStart, FVector& RayEnd)
{
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();
	if (PlayerController)
	{
		FVector WorldLocation, WorldDirection;
		if (PlayerController->DeprojectMousePositionToWorld(WorldLocation, WorldDirection))
		{
			RayStart = WorldLocation;
			float RayDistance = 100000.0f;
			RayEnd = WorldLocation + (WorldDirection * RayDistance);
			return true;
		}
	}

	return false;

}

void APuzzlePawn::HandleOnWorldPress()
{
	if (GrabMode)
	{
		FVector RayStart, RayEnd;

		if (CreateRayFromMouseLocation(RayStart, RayEnd))
		{
			FHitResult HitResult;
			FCollisionQueryParams Params;
			Params.bTraceComplex = true;
			Params.AddIgnoredActor(this);

			if (GetWorld()->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_GameTraceChannel1, Params))
			{
				HandlePieceTouched(HitResult);

				if (CurrentPieceComponent)
				{
					if (CurrentPieceComponent->GetIsLocked())
					{
						//todo?
						//Logic to remove piece
						//RemovingFinalPosition = CurrentPieceComponent->GetRemovingFinalPosition();
						//CurrentPieceComponent->SavePieceInitialRelativeRotation();
					}
					else
					{
						CurrentPieceComponent->SavePieceInitialRelativeRotation();
						if (GetWorld()->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_GameTraceChannel2, Params) || IsPieceCloseToCamera())
						{
							CanMovePiece = true;
						}
						else
						{

							CurrentPieceComponent->InitializeLerpCloseToCameraTimeline(PieceInWorldDistanceFromCamera);
							CanMovePiece = false;
							CurrentPieceComponent->OnLerpToCameraCompletedCallback.AddDynamic(this, &APuzzlePawn::OnPieceLerpCloseToCameraCompletedCallback);
						}
					}
				}

			}

		}
	}
}

void APuzzlePawn::HandlePieceTouched(FHitResult& HitResult)
{
	if (UPuzzlePieceParentComponent* PieceParent = Cast<UPuzzlePieceParentComponent>(HitResult.Component.Get()->GetAttachParent()))
	{
		if (PieceParent->GetIsLocked())
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("Piece is Locked"));

		}
		else
		{
			CurrentPieceComponent = PieceParent;
			if (!OriginalMaterial)
			{
				UMaterialInterface* material = CurrentPieceComponent->GetPieceMesh()->GetMaterial(1);

				if (material)
				{
					OriginalMaterial = material->GetBaseMaterial();
				}
				else
				{
					UE_LOG(LogTemp, Warning, TEXT("Could not find a material to assign to OriginalMaterial"));
				}
			}

			if (PuzzleGameInstance && PuzzleGameInstance->ShowHints)
			{
				CurrentPieceComponent->GetPieceMesh()->SetMaterial(1, HintMaterial);
			}

			CanMovePiece = false;
		}
	}
}

bool APuzzlePawn::IsPieceCloseToCamera()
{
	if (CurrentPieceComponent)
	{
		return FVector::Distance(CurrentPieceComponent->GetComponentLocation(), GetComponentByClass<UCameraComponent>()->GetComponentLocation()) < PieceInWorldDistanceFromCamera;
	}
	return false;
}

float APuzzlePawn::CalculateDistanceFromCurrentPosition()
{
	if (CurrentPieceComponent)
	{
		return FVector::Distance(CurrentPieceComponent->GetComponentLocation(), CurrentPieceComponent->GetParentInitialWorldPositionWithOffset());
	}

	return 0;
}

void APuzzlePawn::DeselectPieceComponent()
{
	if (CurrentPieceComponent)
	{
		CurrentPieceComponent->GetPieceMesh()->SetMaterial(1, OriginalMaterial);
	}

}



// Called every frame
void APuzzlePawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	SetCameraCurrentPosition();

}

void APuzzlePawn::SetCameraCurrentPosition()
{
	if (CameraSpringArm)
	{
		// Pega o valor atual do SocketOffset
		FVector CurrentOffset = CameraSpringArm->SocketOffset;

		// Calcula o novo offset X baseado na direção do movimento e velocidade
		float NewOffsetZ = CurrentOffset.Z + (CameraMovementSpeed * CameraMovementDirection * GetWorld()->GetDeltaSeconds());

		// Define o novo SocketOffset
		CameraSpringArm->SocketOffset = FVector(CurrentOffset.X, CurrentOffset.Y, NewOffsetZ);
	}
}

// Called to bind functionality to input
void APuzzlePawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveCamera", this, &APuzzlePawn::SetCameraMovementDirection);
	PlayerInputComponent->BindAction("LeftMousePressed", IE_Pressed, this, &APuzzlePawn::OnLeftMouseButtonPressed);
	PlayerInputComponent->BindAction("LeftMousePressed", IE_Released, this, &APuzzlePawn::OnLeftMouseButtonReleased);
	PlayerInputComponent->BindAction("RightMousePressed", IE_Pressed, this, &APuzzlePawn::OnRightMouseButtonPressed);
	PlayerInputComponent->BindAction("RightMousePressed", IE_Released, this, &APuzzlePawn::OnRightMouseButtonReleased);
}

void APuzzlePawn::SetCameraMovementDirection(float value)
{
	CameraMovementDirection = value;
}

void APuzzlePawn::SetGrabMode(bool grabMode)
{
	GrabMode = grabMode;
	APlayerController* PlayerController = GetWorld()->GetFirstPlayerController();

	if (PlayerController)
	{
		PlayerController->bShowMouseCursor = GrabMode;
	}
	UE_LOG(LogTemp, Warning, TEXT("SetGrabMode: %s"), grabMode ? TEXT("true") : TEXT("false"));

}

bool APuzzlePawn::GetGrabMode()
{

	return GrabMode;
}

