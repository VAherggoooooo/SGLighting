#include "SGPathTracing.h"

#include "FTaskGraph_SimpleTask.h"
#include "MakeLightmapRDG.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Components/LightComponent.h"
#include "Engine/TextureRenderTarget2D.h"



IMPLEMENT_GLOBAL_SHADER(FSGComputeShader_PT, "/Plugins/SGLighting/Private/SGPathTracing.usf", "MainCS", SF_Compute);


void USGPathTracing::PathTracingInLightmap(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* Position_RT,
	UTextureRenderTarget2D* Normal_RT, UTextureRenderTarget2D* Tangent_RT, ADirectionalLight* MainLight
	, uint8 SampleCount, uint8 depth, float seed, UTextureRenderTarget2D* TestTexture)
{
	
	// check(IsInGameThread());
	//
	// FTexture2DRHIRef RenderTargetRHI = OutputRT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	// FTexture2DRHIRef positionRT = Position_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	// FTexture2DRHIRef normalRT = Normal_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	// FTexture2DRHIRef tangentRT = Tangent_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	//
	//
	// FTexture2DRHIRef testTextureRT = TestTexture->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	//
	// //UE_LOG(LogTemp,Warning, TEXT("texture: %s"), *(RenderTargetRHI->GetName().ToString()));
	//
	// ENQUEUE_RENDER_COMMAND(CaptureCommand)
	// (
	// 	[RenderTargetRHI, positionRT, normalRT, tangentRT, MainLight, SampleCount, depth, seed, testTextureRT](FRHICommandListImmediate &RHICmdList)
	// 	{
	// 		ComputePathTracing(RHICmdList, RenderTargetRHI, positionRT, normalRT, tangentRT, MainLight, SampleCount, depth, seed, testTextureRT);
	// 	}
	// );
}

void USGPathTracing::CreateTaskGraph_SimpleTask(
	const FString& ThreadName,
	UTextureRenderTarget2D* OutputRT,
	UTextureRenderTarget2D* Position_RT,
	UTextureRenderTarget2D* Normal_RT,
	UTextureRenderTarget2D* Tangent_RT,
	ADirectionalLight* MainLight,
	uint8 SampleCount,
	uint8 depth, 
	float seed )
{
	TGraphTask<FTaskGraph_SimpleTask>::CreateTask().ConstructAndDispatchWhenReady(ThreadName, OutputRT, Position_RT, Normal_RT, Tangent_RT,MainLight, SampleCount, depth, seed); // ThreadName 为 FTaskGraph_SimpleTask 构造函数参数
	//TGraphTask<FTaskGraph_SimpleTask>*  GraphTask = TGraphTask<FTaskGraph_SimpleTask>::CreateTask().ConstructAndHold(ThreadName); // 创建任务挂起，等待 unlock() 触发任务执行
	//GraphTask->Unlock();
}

