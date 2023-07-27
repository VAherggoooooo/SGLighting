#include "DataDrivenShaderPlatformInfo.h"
#include "Rendering/SimpleRenderingExample.h"
#include "Engine/TextureRenderTarget2D.h"

#include "PipelineStateCache.h"

#include "GlobalShader.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"
#include "RHIStaticStates.h"
#include "ShaderParameterUtils.h"
#include "PixelShaderUtils.h"

namespace SimpleRenderingExample
{
	/*
	 * Vertex Resource
	 */
	TGlobalResource<FTextureVertexDeclaration> GTextureVertexDeclaration;
	TGlobalResource<FRectangleVertexBuffer> GRectangleVertexBuffer;
	TGlobalResource<FRectangleIndexBuffer> GRectangleIndexBuffer;

	/*
	 * Shader 
	 */
	class FSGComputeShader : public FGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FSGComputeShader);
		SHADER_USE_PARAMETER_STRUCT(FSGComputeShader, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutTexture)
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters &Parameters)
		{
			return RHISupportsComputeShaders(Parameters.Platform);
		}
	};

	class FSimpleRDGGlobalShader : public FGlobalShader
	{
	public:
		SHADER_USE_PARAMETER_STRUCT(FSimpleRDGGlobalShader, FGlobalShader);

		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
			SHADER_PARAMETER_TEXTURE(Texture2D, TextureVal)
			SHADER_PARAMETER_SAMPLER(SamplerState, TextureSampler)
			RENDER_TARGET_BINDING_SLOTS()
		END_SHADER_PARAMETER_STRUCT()

		static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters &Parameters)
		{
			return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
		}
	};

	class FSGVertexShader : public FSimpleRDGGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FSGVertexShader);

		FSGVertexShader() {}

		FSGVertexShader(const ShaderMetaType::CompiledShaderInitializerType &Initializer) : FSimpleRDGGlobalShader(Initializer) {}
	};

	class FSGPixelShader : public FSimpleRDGGlobalShader
	{
	public:
		DECLARE_GLOBAL_SHADER(FSGPixelShader);

		FSGPixelShader() {}

		FSGPixelShader(const ShaderMetaType::CompiledShaderInitializerType &Initializer) : FSimpleRDGGlobalShader(Initializer) {}
	};

	IMPLEMENT_GLOBAL_SHADER(FSGComputeShader, "/Plugins/SGLighting/Private/SimpleComputeShader.usf", "MainCS", SF_Compute);
	IMPLEMENT_GLOBAL_SHADER(FSGVertexShader, "/Plugins/SGLighting/Private/SimplePixelShader.usf", "MainVS", SF_Vertex);
	IMPLEMENT_GLOBAL_SHADER(FSGPixelShader, "/Plugins/SGLighting/Private/SimplePixelShader.usf", "MainPS", SF_Pixel);

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

	void RDGDraw(FRHICommandListImmediate &RHIImmCmdList, FTexture2DRHIRef RenderTargetRHI, FTexture2DRHIRef InTexture)
	{
		check(IsInRenderingThread());

		//Create PooledRenderTarget
		const FRDGTextureDesc& RenderTargetDesc = FRDGTextureDesc::Create2D(RenderTargetRHI->GetSizeXY(), RenderTargetRHI->GetFormat(), FClearValueBinding::Black,  TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV);
		TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;

		//RDG Begin
		FRDGBuilder GraphBuilder(RHIImmCmdList);
		FRDGTextureRef RDGRenderTarget = GraphBuilder.CreateTexture(RenderTargetDesc, TEXT("RDGRenderTarget"));

		//创建参数
		FSGPixelShader::FParameters *Parameters = GraphBuilder.AllocParameters<FSGPixelShader::FParameters>();
		Parameters->TextureVal = InTexture;
		Parameters->TextureSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
		Parameters->RenderTargets[0] = FRenderTargetBinding(RDGRenderTarget, ERenderTargetLoadAction::ENoAction);

		//get shader
		const ERHIFeatureLevel::Type FeatureLevel = GMaxRHIFeatureLevel; //ERHIFeatureLevel::SM5
		FGlobalShaderMap *GlobalShaderMap = GetGlobalShaderMap(FeatureLevel);
		TShaderMapRef<FSGVertexShader> VertexShader(GlobalShaderMap);
		TShaderMapRef<FSGPixelShader> PixelShader(GlobalShaderMap);

		GraphBuilder.AddPass(RDG_EVENT_NAME("RDGDraw"),Parameters,ERDGPassFlags::Raster,
			[Parameters, VertexShader, PixelShader, GlobalShaderMap](FRHICommandList &RHICmdList)
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
				GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
				SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit,0);
				RHICmdList.SetStencilRef(0);
				SetShaderParameters(RHICmdList, PixelShader, PixelShader.GetPixelShader(), *Parameters);

				RHICmdList.SetStreamSource(0, GRectangleVertexBuffer.VertexBufferRHI, 0);
				RHICmdList.DrawIndexedPrimitive(
					GRectangleIndexBuffer.IndexBufferRHI,
					/*BaseVertexIndex=*/0,
					/*MinIndex=*/0,
					/*NumVertices=*/4,
					/*StartIndex=*/0,
					/*NumPrimitives=*/2,
					/*NumInstances=*/1);
			});

		GraphBuilder.QueueTextureExtraction(RDGRenderTarget, &PooledRenderTarget);
		GraphBuilder.Execute();

		//Copy Result To RenderTarget Asset
		RHIImmCmdList.CopyTexture(PooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, RenderTargetRHI->GetTexture2D(), FRHICopyTextureInfo());
	}
} // namespace SimpleRenderingExample

void USimpleRenderingExampleBlueprintLibrary::UseRDGComput(const UObject *WorldContextObject, UTextureRenderTarget2D *OutputRenderTarget)
{
	check(IsInGameThread());

	FTexture2DRHIRef RenderTargetRHI = OutputRenderTarget->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)
	(
		[RenderTargetRHI](FRHICommandListImmediate &RHICmdList) {
			SimpleRenderingExample::RDGCompute(RHICmdList, RenderTargetRHI);
		});
}

void USimpleRenderingExampleBlueprintLibrary::UseRDGDraw(const UObject *WorldContextObject, UTextureRenderTarget2D *OutputRenderTarget, UTexture2D *InTexture)
{
	check(IsInGameThread());

	//两张texture
	FTexture2DRHIRef RenderTargetRHI = OutputRenderTarget->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef InTextureRHI = InTexture->GetResource()->TextureRHI->GetTexture2D();

	ENQUEUE_RENDER_COMMAND(CaptureCommand)
	(
		[RenderTargetRHI, InTextureRHI](FRHICommandListImmediate &RHICmdList)
		{
			SimpleRenderingExample::RDGDraw(RHICmdList, RenderTargetRHI, InTextureRHI);
		}
	);
}