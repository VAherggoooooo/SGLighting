#pragma once
#include "SGPathTracing.generated.h"

UCLASS(MinimalAPI, meta = (ScriptName = "SG Lighting"))
class USGPathTracing : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "SG Lightmap", meta = (WorldContext = "WorldContextObject"))
	static void PathTracingInLightmap(const UObject* WorldContextObject, UTextureRenderTarget2D* Position_RT, UTextureRenderTarget2D* Normal_RT, UTextureRenderTarget2D* Tangent_RT);
};