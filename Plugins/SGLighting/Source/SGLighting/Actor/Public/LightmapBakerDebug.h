// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "LightmapBaker.h"
#include "LightmapBaker.h"
#include "LightmapBakerDebug.generated.h"

/**
 * 
 */
UCLASS()
class SGLIGHTING_API ALightmapBakerDebug : public ALightmapBaker
{
	GENERATED_BODY()

public:
	ALightmapBakerDebug();

	UPROPERTY(VisibleAnywhere)
	TArray<UArrowComponent*> arrs;

	UFUNCTION(CallInEditor)
	void DebugSGDirs();

	virtual void Tick(float DeltaSeconds) override;
	virtual void BeginPlay() override;
};
