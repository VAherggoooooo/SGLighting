#include "SGLighting/Rendering/MakeLightmapRDG.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "Engine/TextureRenderTarget2D.h"

#include "PipelineStateCache.h"

#include "GlobalShader.h"
#include "HLSLTypeAliases.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "RHIStaticStates.h"
#include "ShaderParameterUtils.h"
#include "PixelShaderUtils.h"


void UMakeLightmapBlueprintLibrary::UseRDGComput(const UObject *WorldContextObject, UTextureRenderTarget2D *OutputRenderTarget)
{
	check(IsInGameThread());

	FTexture2DRHIRef RenderTargetRHI = OutputRenderTarget->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)
	(
		[RenderTargetRHI](FRHICommandListImmediate &RHICmdList)
		{
			RDGCompute(RHICmdList, RenderTargetRHI);
		}
	);
}

void UMakeLightmapBlueprintLibrary::UseRDGDraw(const UObject* WorldContextObject, UTextureRenderTarget2D* Output_Position_RT, UTextureRenderTarget2D* Output_Normal_RT, UTextureRenderTarget2D* Output_Tangent_RT)
{
	check(IsInGameThread());

	//两张texture
	FTexture2DRHIRef positionRT = Output_Position_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef normalRT = Output_Normal_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef tangentRT = Output_Tangent_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)
	(
		[positionRT, normalRT, tangentRT](FRHICommandListImmediate &RHICmdList)
		{
			RDGDraw(RHICmdList, positionRT, normalRT, tangentRT);
		}
	);
}

void ULightmapCollect::Init(UBVHData* _bvhData)
{
	if(_bvhData == nullptr)
	{
		_bvhData = NewObject<UBVHData>();
	}
	this->BVHData = _bvhData;
}

void ULightmapCollect::UseRDGDraw(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderTarget, UTexture2D* InTexture, int32 Size)
{
	// check(IsInGameThread());
	//
	// //两张texture
	// FTexture2DRHIRef RenderTargetRHI = OutputRenderTarget->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	// FTexture2DRHIRef InTextureRHI = InTexture->GetResource()->TextureRHI->GetTexture2D();
	//
	// ENQUEUE_RENDER_COMMAND(CaptureCommand)
	// (
	// 	[RenderTargetRHI, InTextureRHI, Size](FRHICommandListImmediate &RHICmdList)
	// 	{
	// 		RDGDraw(RHICmdList, RenderTargetRHI, InTextureRHI, Size);
	// 	}
	// );
}


TGlobalResource<FTextureVertexDeclaration> GTextureVertexDeclaration;
TGlobalResource<FRectangleVertexBuffer> GRectangleVertexBuffer;
TGlobalResource<FRectangleIndexBuffer> GRectangleIndexBuffer;




//绑定shader
IMPLEMENT_GLOBAL_SHADER(FSGComputeShader, "/Plugins/SGLighting/Private/SimpleComputeShader.usf", "MainCS", SF_Compute);

IMPLEMENT_GLOBAL_SHADER(FSGVertexShader, "/Plugins/SGLighting/Private/GenerateBakePointShader.usf", "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FSGPixelPositionShader, "/Plugins/SGLighting/Private/GenerateBakePointShader.usf", "MainPS_Position", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FSGPixelNormalShader, "/Plugins/SGLighting/Private/GenerateBakePointShader.usf", "MainPS_Normal", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FSGPixelTangentShader, "/Plugins/SGLighting/Private/GenerateBakePointShader.usf", "MainPS_Tangent", SF_Pixel);

/*
 * Render Function 
 */
void RDGCompute(FRHICommandListImmediate &RHIImmCmdList, FTexture2DRHIRef RenderTargetRHI)
{
	check(IsInRenderingThread());

	//Create RenderTargetDesc
	const FRDGTextureDesc& RenderTargetDesc = FRDGTextureDesc::Create2D(RenderTargetRHI->GetSizeXY(),RenderTargetRHI->GetFormat(), FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV);
	
	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;

	//RDG Begin
	FRDGBuilder GraphBuilder(RHIImmCmdList);
	FRDGTextureRef RDGRenderTarget = GraphBuilder.CreateTexture(RenderTargetDesc, TEXT("RDGRenderTarget"));
	
	FSGComputeShader::FParameters *Parameters = GraphBuilder.AllocParameters<FSGComputeShader::FParameters>();
	FRDGTextureUAVDesc UAVDesc(RDGRenderTarget);
	Parameters->OutTexture = GraphBuilder.CreateUAV(UAVDesc);

	//Get ComputeShader From GlobalShaderMap
	const ERHIFeatureLevel::Type FeatureLevel = GMaxRHIFeatureLevel; //ERHIFeatureLevel::SM5
	FGlobalShaderMap *GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
	TShaderMapRef<FSGComputeShader> ComputeShader(GlobalShaderMap);

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

void RDGDraw(FRHICommandListImmediate &RHIImmCmdList, FTexture2DRHIRef PositonRHI, FTexture2DRHIRef NormalRHI, FTexture2DRHIRef TangentRHI)
{
	check(IsInRenderingThread());

	GRectangleVertexBuffer.InitRHI();
	GTextureVertexDeclaration.InitRHI();
	GRectangleIndexBuffer.InitRHI();

	//UE_LOG(LogTemp, Warning, TEXT("%d"), GRectangleVertexBuffer.VertexNum);

	DrawToRT(RHIImmCmdList, PositonRHI, OutRTType::PositionWS);
	DrawToRT(RHIImmCmdList, NormalRHI, OutRTType::NormalWS);
	DrawToRT(RHIImmCmdList, TangentRHI, OutRTType::TangentWS);
}

void DrawToRT(FRHICommandListImmediate &RHIImmCmdList, FTexture2DRHIRef RTRHI, OutRTType Type)
{
	const FRDGTextureDesc& RenderTargetDesc = FRDGTextureDesc::Create2D(RTRHI->GetSizeXY(), RTRHI->GetFormat(), FClearValueBinding::Black,  TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV);
	FRDGBuilder GraphBuilder(RHIImmCmdList);
	FRDGTextureRef RDGRenderTarget = GraphBuilder.CreateTexture(RenderTargetDesc, TEXT("RDGRenderTarget"));

	FRDGGlobalShader::FParameters *Parameters = GraphBuilder.AllocParameters<FSGPixelShader::FParameters>();
	Parameters->RenderTargets[0] = FRenderTargetBinding(RDGRenderTarget, ERenderTargetLoadAction::ENoAction);
	UE::Math::TMatrix<float> M_Matrix = GRectangleVertexBuffer.GetMMatrix();
	Parameters->M_Matrix = FMatrix44f(M_Matrix);
	Parameters->M_Matrix_Invers_Trans = FMatrix44f(M_Matrix.Inverse().GetTransposed());

	
	//get shader
	const ERHIFeatureLevel::Type FeatureLevel = GMaxRHIFeatureLevel; //ERHIFeatureLevel::SM5
	FGlobalShaderMap *GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;
	TShaderMapRef<FSGVertexShader> VertexShader(GlobalShaderMap);
	TShaderMapRef<FSGPixelPositionShader> PixelShader_Position(GlobalShaderMap);
	TShaderMapRef<FSGPixelNormalShader> PixelShader_Normal(GlobalShaderMap);
	TShaderMapRef<FSGPixelTangentShader> PixelShader_Tangent(GlobalShaderMap);
	
	if(Type == OutRTType::NormalWS)
	{
		GraphBuilder.AddPass(RDG_EVENT_NAME("RDGDraw"),Parameters,ERDGPassFlags::Raster,
		[Parameters, VertexShader, PixelShader_Normal, GlobalShaderMap](FRHICommandList &RHICmdList)
			{
				FRHITexture2D *RT = Parameters->RenderTargets[0].GetTexture()->GetRHI()->GetTexture2D();
				RHICmdList.SetViewport(0, 0, 0.0f, RT->GetSizeX(), RT->GetSizeY(), 1.0f);
	
				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTextureVertexDeclaration.VertexDeclarationRHI;
	
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader_Normal.GetPixelShader();
				
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit,0);
				RHICmdList.SetStencilRef(0);
				
				SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), *Parameters);
				SetShaderParameters(RHICmdList, PixelShader_Normal, PixelShader_Normal.GetPixelShader(), *Parameters);
	
				RHICmdList.SetStreamSource(0, GRectangleVertexBuffer.VertexBufferRHI, 0);
	
				RHICmdList.DrawIndexedPrimitive(
					GRectangleIndexBuffer.IndexBufferRHI,
					/*BaseVertexIndex=*/0,
					/*MinIndex=*/0,
					/*NumVertices=*/GRectangleVertexBuffer.VertexNum,
					/*StartIndex=*/0,
					/*NumPrimitives=*/GRectangleIndexBuffer.PrimitiveNum,
					/*NumInstances=*/1);
			}
		);
	}
	else if(Type == OutRTType::TangentWS)
	{
		GraphBuilder.AddPass(RDG_EVENT_NAME("RDGDraw"),Parameters,ERDGPassFlags::Raster,
		[Parameters, VertexShader, PixelShader_Tangent, GlobalShaderMap](FRHICommandList &RHICmdList)
			{
				FRHITexture2D *RT = Parameters->RenderTargets[0].GetTexture()->GetRHI()->GetTexture2D();
				RHICmdList.SetViewport(0, 0, 0.0f, RT->GetSizeX(), RT->GetSizeY(), 1.0f);
	
				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTextureVertexDeclaration.VertexDeclarationRHI;
	
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader_Tangent.GetPixelShader();
				
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit,0);
				RHICmdList.SetStencilRef(0);
				
				SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), *Parameters);
				SetShaderParameters(RHICmdList, PixelShader_Tangent, PixelShader_Tangent.GetPixelShader(), *Parameters);
	
				RHICmdList.SetStreamSource(0, GRectangleVertexBuffer.VertexBufferRHI, 0);
	
				RHICmdList.DrawIndexedPrimitive(
					GRectangleIndexBuffer.IndexBufferRHI,
					/*BaseVertexIndex=*/0,
					/*MinIndex=*/0,
					/*NumVertices=*/GRectangleVertexBuffer.VertexNum,
					/*StartIndex=*/0,
					/*NumPrimitives=*/GRectangleIndexBuffer.PrimitiveNum,
					/*NumInstances=*/1);
			}
		);
	}
	else
	{
		GraphBuilder.AddPass(RDG_EVENT_NAME("RDGDraw"),Parameters,ERDGPassFlags::Raster,
		[Parameters, VertexShader, PixelShader_Position, GlobalShaderMap](FRHICommandList &RHICmdList)
			{
				FRHITexture2D *RT = Parameters->RenderTargets[0].GetTexture()->GetRHI()->GetTexture2D();
				RHICmdList.SetViewport(0, 0, 0.0f, RT->GetSizeX(), RT->GetSizeY(), 1.0f);
	
				FGraphicsPipelineStateInitializer GraphicsPSOInit;
				RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
				GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, CF_Always>::GetRHI();
				GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
				GraphicsPSOInit.RasterizerState = TStaticRasterizerState<>::GetRHI();
				GraphicsPSOInit.PrimitiveType = PT_TriangleList;
				GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GTextureVertexDeclaration.VertexDeclarationRHI;
	
				GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader_Position.GetPixelShader();
				
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit,0);
				RHICmdList.SetStencilRef(0);
				
				SetShaderParameters(RHICmdList, VertexShader, VertexShader.GetVertexShader(), *Parameters);
				SetShaderParameters(RHICmdList, PixelShader_Position, PixelShader_Position.GetPixelShader(), *Parameters);
	
				RHICmdList.SetStreamSource(0, GRectangleVertexBuffer.VertexBufferRHI, 0);
	
				RHICmdList.DrawIndexedPrimitive(
					GRectangleIndexBuffer.IndexBufferRHI,
					/*BaseVertexIndex=*/0,
					/*MinIndex=*/0,
					/*NumVertices=*/GRectangleVertexBuffer.VertexNum,
					/*StartIndex=*/0,
					/*NumPrimitives=*/GRectangleIndexBuffer.PrimitiveNum,
					/*NumInstances=*/1);
			}
		);
	}

	GraphBuilder.QueueTextureExtraction(RDGRenderTarget, &PooledRenderTarget);
	GraphBuilder.Execute();
	RHIImmCmdList.CopyTexture(PooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, RTRHI->GetTexture2D(), FRHICopyTextureInfo());//Copy Result To RenderTarget Asset
}
