// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "AMainModel.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class PUZZLE3D_API UAMainModel : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UAMainModel();

	AActor* ModelActorReference;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	float ExplosionRadius;


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Explosion")
	int InitialPieces = 2;

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//UFUNCTION(BlueprintCallable) // Esta macro torna o método acessível em Blueprint
	//void CheckPosition(UStaticMeshComponent* mesh);

private:
	void Explode();
	void GetRandomPointInSphere(FVector& OutPosition, const FVector& Center);

};
