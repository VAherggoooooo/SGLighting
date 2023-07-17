#pragma once

#include "CoreMinimal.h"
#include "MeshCollecter.h"
#include "LevelManager.generated.h"

UCLASS()
class SGLIGHTING_API ULevelManager : public UObject
{
	GENERATED_BODY()

public:
	ULevelManager();
	~ULevelManager();

	virtual UMeshCollecter* GetMeshCollector() const { return MeshCollecter == nullptr? nullptr:MeshCollecter; } 
private:
	UMeshCollecter* MeshCollecter;
};