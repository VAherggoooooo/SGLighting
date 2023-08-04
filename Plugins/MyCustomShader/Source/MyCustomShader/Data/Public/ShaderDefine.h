#pragma once
#include "DataDrivenShaderPlatformInfo.h"
#include "ShaderParameterStruct.h"

//
class FMyGlobalShaderBase : public FGlobalShader
{
public:
	SHADER_USE_PARAMETER_STRUCT(FMyGlobalShaderBase, FGlobalShader);
		BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )//添加要传入shader的全局参数
				SHADER_PARAMETER_TEXTURE(Texture2D, InputTexture)
				SHADER_PARAMETER_SAMPLER(SamplerState, InputTextureSampler)
			RENDER_TARGET_BINDING_SLOTS()
		END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters &Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}
};

//
class FMyVertexShader : public FMyGlobalShaderBase
{
public:
	DECLARE_GLOBAL_SHADER(FMyVertexShader);

	FMyVertexShader() {}

	FMyVertexShader(const ShaderMetaType::CompiledShaderInitializerType &Initializer) : FMyGlobalShaderBase(Initializer) {}
};


//
class FMyPixelShader : public FMyGlobalShaderBase
{
public:
	DECLARE_GLOBAL_SHADER(FMyPixelShader);

	FMyPixelShader() {}

	FMyPixelShader(const ShaderMetaType::CompiledShaderInitializerType &Initializer) : FMyGlobalShaderBase(Initializer) {}
};