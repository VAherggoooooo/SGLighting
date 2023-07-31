// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "BakePoint.generated.h"

UCLASS()
class SGLIGHTING_API ABakePoint : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABakePoint();

	UPROPERTY(VisibleDefaultsOnly)
	USceneComponent* Root;
	
	UPROPERTY(VisibleDefaultsOnly)
	UInstancedStaticMeshComponent* StaticMeskComp;


	TArray<FVector> TransformList;

	virtual void OnConstruction(const FTransform& Transform) override;
	
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
};
