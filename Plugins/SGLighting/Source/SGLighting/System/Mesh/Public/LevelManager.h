#pragma once

#include "CoreMinimal.h"
#include "BVHData.h"
#include "MeshCollecter.h"
#include "SGLighting/Rendering/MakeLightmapRDG.h"
#include "LevelManager.generated.h"

UCLASS()
class SGLIGHTING_API ULevelManager : public UObject
{
	GENERATED_BODY()

public:
	ULevelManager();
	~ULevelManager();

	virtual UMeshCollecter* GetMeshCollector() const { return MeshCollecter == nullptr? nullptr:MeshCollecter; } 
	virtual UBVHData* GetBVHData() const { return BVHData == nullptr? nullptr:BVHData; } 
	virtual ULightmapCollect* GetLightmapCollect() const { return LightmapCollect == nullptr? nullptr:LightmapCollect; } 
private:
	UMeshCollecter* MeshCollecter;
	UBVHData* BVHData;
	ULightmapCollect* LightmapCollect;
};