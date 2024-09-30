// Fill out your copyright notice in the Description page of Project Settings.

#include "PuzzlePawn.h"

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
	FVector RayStart, RayEnd;

	if (CreateRayFromMouseLocation(RayStart, RayEnd))
	{
		FHitResult HitResult;
		FCollisionQueryParams Params;
		Params.bTraceComplex = true;

		if (GetWorld()->LineTraceSingleByChannel(HitResult, RayStart, RayEnd, ECC_GameTraceChannel3, Params))
		{
			HandleOnBoardPress();
		}
		else
		{
			HandleOnWorldPress();
		}

	}

}

void APuzzlePawn::OnLeftMouseButtonReleased()
{
	if (GrabMode && CurrentPieceComponent)
	{
		if (CalculateDistanceFromCurrentPosition() < CorrectPositionTolerance)
		{
			LastGrabbedPieceComponent = CurrentPieceComponent;
			CurrentPieceComponent->SetIsLocked(true);
			CurrentPieceComponent = nullptr;
			InitialWorldLerpLocation2 = LastGrabbedPieceComponent->GetAttachParent()->GetComponentLocation();
			SetGrabMode(false);

		}
		else
		{
			DeselectPieceComponent();
			CurrentPieceComponent = nullptr;
		}
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
}

float APuzzlePawn::CalculateDistanceFromCurrentPosition()
{
	if (CurrentPieceComponent)
	{
		return FVector::Distance(CurrentPieceComponent->GetAttachParent()->GetComponentLocation(), CurrentPieceComponent->GetParentInitialWorldPositionWithOffset());
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

void APuzzlePawn::HandleOnBoardPress()
{
}

void APuzzlePawn::UpdatePlacingPiece(float Alpha)
{
}

void APuzzlePawn::UpdatePlacingPieceWithOffset(float Alpha)
{
}

void APuzzlePawn::OnTimelineFinished()
{
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
	PlayerInputComponent->BindAction("LeftMouseReleased", IE_Released, this, &APuzzlePawn::OnLeftMouseButtonReleased);

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
}

bool APuzzlePawn::GetGrabMode()
{
	return GrabMode;
}

