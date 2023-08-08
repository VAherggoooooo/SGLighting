#include "SGPathTracing.h"

#include "MakeLightmapRDG.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Engine/TextureRenderTarget2D.h"



IMPLEMENT_GLOBAL_SHADER(FSGComputeShader_PT, "/Plugins/SGLighting/Private/SGPathTracing.usf", "MainCS", SF_Compute);


void USGPathTracing::PathTracingInLightmap(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* Position_RT,
	UTextureRenderTarget2D* Normal_RT, UTextureRenderTarget2D* Tangent_RT)
{
	check(IsInGameThread());

	FTexture2DRHIRef RenderTargetRHI = OutputRT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef positionRT = Position_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef normalRT = Normal_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef tangentRT = Tangent_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)
	(
		[RenderTargetRHI, positionRT, normalRT, tangentRT](FRHICommandListImmediate &RHICmdList)
		{
			ComputePathTracing(RHICmdList, RenderTargetRHI, positionRT, normalRT, tangentRT);
		}
	);
}

void ComputePathTracing(FRHICommandListImmediate &RHIImmCmdList, FTexture2DRHIRef RenderTargetRHI, FTexture2DRHIRef Position_RT, FTexture2DRHIRef Normal_RT, FTexture2DRHIRef Tangent_RT)
{
	check(IsInRenderingThread());

	//Create RenderTargetDesc
	const FRDGTextureDesc& RenderTargetDesc = FRDGTextureDesc::Create2D(RenderTargetRHI->GetSizeXY(),RenderTargetRHI->GetFormat(), FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV);
	
	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;

	//RDG Begin
	FRDGBuilder GraphBuilder(RHIImmCmdList);
	FRDGTextureRef RDGRenderTarget = GraphBuilder.CreateTexture(RenderTargetDesc, TEXT("RDGRenderTarget"));
	
	FSGComputeShader_PT::FParameters *Parameters = GraphBuilder.AllocParameters<FSGComputeShader_PT::FParameters>();
	FRDGTextureUAVDesc UAVDesc(RDGRenderTarget);
	Parameters->OutTexture = GraphBuilder.CreateUAV(UAVDesc);
	Parameters->InPosition = Position_RT;
	Parameters->InPositionSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	Parameters->InNormal = Normal_RT;
	Parameters->InNormalSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	Parameters->InTangent = Tangent_RT;
	Parameters->InTangentSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();

	FRDGBufferRef TriBuffer = CreateStructuredBuffer(GraphBuilder, TEXT("TriangleDataBuffer"), GRectangleVertexBuffer.SceneMeshTriangles,ERDGInitialDataFlags::NoCopy);//GRectangleVertexBuffer.MeshTriangles
	Parameters->TriangleBuffer = GraphBuilder.CreateSRV(TriBuffer);
	Parameters->TriangleNum = GRectangleVertexBuffer.SceneMeshTriangles.Num();

	//Get ComputeShader From GlobalShaderMap
	const ERHIFeatureLevel::Type FeatureLevel = GMaxRHIFeatureLevel; //ERHIFeatureLevel::SM5
	FGlobalShaderMap *GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
	TShaderMapRef<FSGComputeShader_PT> ComputeShader(GlobalShaderMap);

	//Compute Thread Group Count
	FIntVector ThreadGroupCount(
		RenderTargetRHI->GetSizeX() / 32,
		RenderTargetRHI->GetSizeY() / 32,
		1);

	GraphBuilder.AddPass(
		RDG_EVENT_NAME("RDGCompute"),
		Parameters,
		ERDGPassFlags::Compute,
		[Parameters, ComputeShader, ThreadGroupCount](FRHICommandList &RHICmdList) {
			FComputeShaderUtils::Dispatch(RHICmdList, ComputeShader, *Parameters, ThreadGroupCount);
		});

	GraphBuilder.QueueTextureExtraction(RDGRenderTarget, &PooledRenderTarget);
	GraphBuilder.Execute();

	//Copy Result To RenderTarget Asset
	RHIImmCmdList.CopyTexture(PooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, RenderTargetRHI->GetTexture2D(), FRHICopyTextureInfo());
}