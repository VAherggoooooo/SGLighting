#pragma once
#include "DataDrivenShaderPlatformInfo.h"
#include "GlobalShader.h"
#include "MathUtil.h"
#include "ShaderParameterStruct.h"
#include "SG_Data.h"
//#include "Materials/MaterialInstanceDynamic.h"
#include "SGPathTracing.generated.h"




UCLASS(MinimalAPI, meta = (ScriptName = "SG Lighting"))
class USGPathTracing : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:	
	UFUNCTION(BlueprintCallable, Category = "SG Lightmap", meta = (WorldContext = "WorldContextObject"))
	static void PathTracingInLightmap(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* Position_RT, UTextureRenderTarget2D* Normal_RT, UTextureRenderTarget2D* Tangent_RT, ADirectionalLight* MainLight, uint8 SampleCount, uint8 depth, float seed, UTextureRenderTarget2D* TestTexture);

	UFUNCTION(BlueprintCallable)
	static void CreateTaskGraph_SimpleTask(const FString& ThreadName, UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* Position_RT, UTextureRenderTarget2D* Normal_RT, UTextureRenderTarget2D* Tangent_RT, ADirectionalLight* MainLight, uint8 SampleCount = 200, uint8 depth = 6, float seed = 0.0f);

	
};




class FSGComputeShader_PT : public FGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSGComputeShader_PT);
	SHADER_USE_PARAMETER_STRUCT(FSGComputeShader_PT, FGlobalShader);

	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, OutTexture)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, TestTexture)
	
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SG1)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SG2)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SG3)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SG4)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SG5)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SG6)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SG7)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SG8)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SG9)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SG10)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SG11)
		SHADER_PARAMETER_RDG_TEXTURE_UAV(RWTexture2D<float4>, SG12)
	
		SHADER_PARAMETER_TEXTURE(Texture2D, InPosition)
		SHADER_PARAMETER_SAMPLER(SamplerState, InPositionSampler)

		SHADER_PARAMETER_TEXTURE(Texture2D, InNormal)
		SHADER_PARAMETER_SAMPLER(SamplerState, InNormalSampler)

		SHADER_PARAMETER_TEXTURE(Texture2D, Albedo)
		SHADER_PARAMETER_SAMPLER(SamplerState, AlbedoSampler)
	

		SHADER_PARAMETER_TEXTURE(Texture2D, InTangent)
		SHADER_PARAMETER_SAMPLER(SamplerState, InTangentSampler)

		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FMeshTriangle>, TriangleBuffer)
		SHADER_PARAMETER(int, TriangleNum)
	
		SHADER_PARAMETER(int, SampleCount)
		SHADER_PARAMETER(int, depth)
		SHADER_PARAMETER(float, seed)
		SHADER_PARAMETER(float, MaxFallOff)
		SHADER_PARAMETER(float, Roughness)

		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FMainLight>, MainLightBuffer)
		SHADER_PARAMETER_RDG_BUFFER_SRV(StructuredBuffer<FSG>, SGBuffer)

	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters &Parameters)
	{
		return RHISupportsComputeShaders(Parameters.Platform);
	}
};


