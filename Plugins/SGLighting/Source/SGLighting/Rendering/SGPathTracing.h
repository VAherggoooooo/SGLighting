#pragma once
#include "DataDrivenShaderPlatformInfo.h"
#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "SGPathTracing.generated.h"

UCLASS(MinimalAPI, meta = (ScriptName = "SG Lighting"))
class USGPathTracing : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "SG Lightmap", meta = (WorldContext = "WorldContextObject"))
	static void PathTracingInLightmap(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* Position_RT, UTextureRenderTarget2D* Normal_RT, UTextureRenderTarget2D* Tangent_RT);
};


void ComputePathTracing(FRHICommandListImmediate &RHIImmCmdList, FTexture2DRHIRef RenderTargetRHI, FTexture2DRHIRef Position_RT, FTexture2DRHIRef Normal_RT, FTexture2DRHIRef Tangent_RT);


class FSGComputeShader_PT : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSGComputeShader_PT);
	SHADER_USE_PARAMETER_STRUCT(FSGComputeShader_PT, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutTexture)
		SHADER_PARAMETER_TEXTURE(Texture2D, InPosition)
		SHADER_PARAMETER_SAMPLER(SamplerState, InPositionSampler)

		SHADER_PARAMETER_TEXTURE(Texture2D, InNormal)
		SHADER_PARAMETER_SAMPLER(SamplerState, InNormalSampler)

		SHADER_PARAMETER_TEXTURE(Texture2D, InTangent)
		SHADER_PARAMETER_SAMPLER(SamplerState, InTangentSampler)

	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters &Parameters)
	{
		return RHISupportsComputeShaders(Parameters.Platform);
	}
};