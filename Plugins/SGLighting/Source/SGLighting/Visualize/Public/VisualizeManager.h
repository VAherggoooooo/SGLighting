#pragma once

#include "VisualizeManager.generated.h"

UCLASS(MinimalAPI, meta = (ScriptName = "Visualize Manager"))
class UVisualLizeManager : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "Visualize Manager", meta = (WorldContext = "WorldContextObject"))
	static void GetVisualBakePointsFromRT(const UObject* WorldContextObject, UTextureRenderTarget2D* InRT);

	UFUNCTION(BlueprintCallable, Category = "Visualize Manager", meta = (WorldContext = "WorldContextObject"))
	static void GetVisualBakePointLocationsFromRT(const UObject* WorldContextObject, UTextureRenderTarget2D* InRT, TArray<FVector>& Locations);
};