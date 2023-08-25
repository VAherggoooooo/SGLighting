#include "SGLighting/Rendering/MakeLightmapRDG.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "PipelineStateCache.h"

#include "GlobalShader.h"
#include "HLSLTypeAliases.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "RHIStaticStates.h"
#include "PixelShaderUtils.h"
#include "SGLighting/Actor/Public/LightmapBaker.h"



void ULightmapCollect::Init(UBVHData* _bvhData)
{
	if(_bvhData == nullptr)
	{
		_bvhData = NewObject<UBVHData>();
	}
	this->BVHData = _bvhData;
}


TGlobalResource<FTextureVertexDeclaration> GTextureVertexDeclaration;
TGlobalResource<FRectangleVertexBuffer> GRectangleVertexBuffer;
TGlobalResource<FRectangleIndexBuffer> GRectangleIndexBuffer;



//绑定shader
IMPLEMENT_GLOBAL_SHADER(FSGVertexShader, "/Plugins/SGLighting/Private/GenerateBakePointShader.usf", "MainVS", SF_Vertex);
IMPLEMENT_GLOBAL_SHADER(FSGPixelPositionShader, "/Plugins/SGLighting/Private/GenerateBakePointShader.usf", "MainPS_Position", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FSGPixelNormalShader, "/Plugins/SGLighting/Private/GenerateBakePointShader.usf", "MainPS_Normal", SF_Pixel);
IMPLEMENT_GLOBAL_SHADER(FSGPixelTangentShader, "/Plugins/SGLighting/Private/GenerateBakePointShader.usf", "MainPS_Tangent", SF_Pixel);

/*
 * Render Function 
 */


void RDGDraw(FRHICommandListImmediate &RHIImmCmdList, FTexture2DRHIRef PositonRHI, FTexture2DRHIRef NormalRHI, FTexture2DRHIRef TangentRHI)
{
	check(IsInRenderingThread());

	GRectangleVertexBuffer.InitRHI();
	GTextureVertexDeclaration.InitRHI();
	GRectangleIndexBuffer.InitRHI();

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
