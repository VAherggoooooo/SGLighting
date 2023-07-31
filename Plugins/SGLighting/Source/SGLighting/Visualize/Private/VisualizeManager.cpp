#include "SGLighting/Visualize/Public/VisualizeManager.h"

#include "AnalyticsPropertyStore.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "SGLighting/Visualize/Public/BakePoint.h"


void UVisualLizeManager::GetVisualBakePointsFromRT(const UObject* WorldContextObject, UTextureRenderTarget2D* InRT)
{
	check(InRT != nullptr);

	FRenderTarget* RenderTarget = InRT->GameThread_GetRenderTargetResource();
	TArray<FFloat16Color> FloatColors;
	RenderTarget->ReadFloat16Pixels(FloatColors);

	UWorld* World = WorldContextObject->GetWorld();
	ABakePoint* bp = World->SpawnActor<ABakePoint>(ABakePoint::StaticClass());
	
	for(int32 i = 0; i < FloatColors.Num(); i += 1)
	{
		FFloat16Color col = FloatColors[i];
		if(col.A.GetFloat() > 0.0f)
		{
			// //TODO: 使用多线程或GPU Instance
			// //UWorld* World = GEditor->GetEditorWorldContext().World();//editor使用
			// UWorld* World = WorldContextObject->GetWorld();
			// ABakePoint* bp = World->SpawnActor<ABakePoint>(ABakePoint::StaticClass());
			//
			// bp->SetActorLocation(FVector(col.R.GetFloat(), col.G.GetFloat(), col.B.GetFloat()));
			// bp->SetFolderPath(FName("BakePoints"));

			bp->TransformList.Add(FVector(col.R.GetFloat(), col.G.GetFloat(), col.B.GetFloat()));
		}
	}
}

void UVisualLizeManager::GetVisualBakePointLocationsFromRT(const UObject* WorldContextObject, UTextureRenderTarget2D* InRT, TArray<FVector>& Locations)
{
	check(InRT != nullptr);

	FRenderTarget* RenderTarget = InRT->GameThread_GetRenderTargetResource();
	TArray<FFloat16Color> FloatColors;
	RenderTarget->ReadFloat16Pixels(FloatColors);
	Locations.Empty();
	for(int32 i = 0; i < FloatColors.Num(); i += 1)
	{
		FFloat16Color col = FloatColors[i];
		if(col.A.GetFloat() > 0.0f)
		{
			Locations.Add(FVector(col.R.GetFloat(), col.G.GetFloat(), col.B.GetFloat()));
		}
	}
}
