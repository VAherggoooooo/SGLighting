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
	void Init();
	virtual TArray<AActor*> GetAllActors(bool bPrintName = false);
	virtual TArray<AActor*> GetAllStaticMeshActors(bool bPrintName = false);
	virtual TArray<UStaticMeshComponent*> GetAllStaticMeshComponents(bool bPrintName = false, bool bStatic = false);
	virtual TArray<UStaticMesh*> GetAllStaticMeshesInLevel(bool bPrintName = false, bool bStatic = true);
};
