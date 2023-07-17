// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "MeshCollecter.generated.h"

/**
 * 
 */
UCLASS()
class SGLIGHTING_API UMeshCollecter : public UObject
{
	GENERATED_BODY()

public:
	UMeshCollecter();
	~UMeshCollecter(){}
	virtual TArray<AActor*> GetAllActors(bool bPrintName = false);
	virtual TArray<AStaticMeshActor*> GetAllStaticMeshActors(bool bPrintName = false);
};
