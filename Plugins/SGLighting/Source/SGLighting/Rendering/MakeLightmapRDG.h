#pragma once
#include "RenderGraph.h"
#include "SGLighting/System/Mesh/Public/MeshCollecter.h"
#include "CoreMinimal.h"
#include "DataDrivenShaderPlatformInfo.h"
#include "GlobalShader.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Math/TransformCalculus3D.h"
#include "SGLighting/System/Mesh/Public/BVHData.h"
#include "ProceduralMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "MakeLightmapRDG.generated.h"

class FRHICommandListImmediate;
class FRDGGlobalShader;
struct IPooledRenderTarget;

enum OutRTType
{
	None = 0,
	PositionWS,
	NormalWS,
	TangentWS,
};

//blurprint node 函数作为起点
UCLASS(MinimalAPI, meta = (ScriptName = "SimpleRenderingExample"))
class UMakeLightmapBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "SG Lightmap", meta = (WorldContext = "WorldContextObject"))
	static void UseRDGComput(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderTarget);

	UFUNCTION(BlueprintCallable, Category = "SG Lightmap", meta = (WorldContext = "WorldContextObject"))
	static void UseRDGDraw(const UObject* WorldContextObject, UTextureRenderTarget2D* Output_Position_RT, UTextureRenderTarget2D* Output_Normal_RT, UTextureRenderTarget2D* Output_Tangent_RT);
};


UCLASS()
class ULightmapCollect : public UObject
{
	GENERATED_BODY()

public:
	void Init(UBVHData* _bvhData);
	void UseRDGDraw(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderTarget, UTexture2D* InTexture, int32 Size);
	UBVHData* GetBVHData() const { return BVHData == nullptr? nullptr:BVHData; }
private:
	UBVHData* BVHData;
};




struct FTextureVertex
{
	FVector4f Position;
	FVector2f UV;
	FVector4f Position_OS;
	FVector4f Normal;
	FVector4f Tangent;
};


/**
 * @brief 顶点buffer, 设置的不同序号顶点的position, uv
 */
class FRectangleVertexBuffer : public FVertexBuffer
{
// private:
// 	UBVHData* BVHData;
public:
	int32 VertexNum = 0;
	TArray<FMeshTriangle> SceneMeshTriangles;
	
	/** Initialize the RHI for this rendering resource */
	void InitRHI() override
	{
		UBVHData* BVHData = NewObject<UBVHData>();
		BVHData->Init(nullptr);
		BVHData->GetSceneData();

		if(BVHData->VerticeIDs.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("No Triangle Data"));
			return;
		}
		
		TResourceArray<FTextureVertex, VERTEXBUFFER_ALIGNMENT> Vertices;

		VertexNum = BVHData->VerticeIDs.Num();
		Vertices.SetNumUninitialized(VertexNum);
		SceneMeshTriangles = BVHData->Triangles;
		
		
		
		//UE_LOG(LogTemp, Warning, TEXT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"));
		for (int32 i = 0; i < BVHData->VerticeIDs.Num(); i++)
		{
			Vertices[i].Position = FVector4f((BVHData->UVs2[i].X * 2.0f) - 1.0f, ((1 - BVHData->UVs2[i].Y) * 2.0f) - 1.0f, 0, 1);//右下角
			Vertices[i].UV = FVector2f(BVHData->UVs2[i].X, BVHData->UVs2[i].Y);
			Vertices[i].Position_OS = FVector4f(BVHData->VerticePositions[i].X, BVHData->VerticePositions[i].Y, BVHData->VerticePositions[i].Z, 1.0f);
			Vertices[i].Normal = FVector4f(BVHData->Normals[i].X, BVHData->Normals[i].Y, BVHData->Normals[i].Z, 0.0f);
			Vertices[i].Tangent = FVector4f(BVHData->Tangents[i].TangentX.X, BVHData->Tangents[i].TangentX.Y, BVHData->Tangents[i].TangentX.Z, 0.0f);
			//Vertices[i].Position_OS = FVector4f(1,1,1,1);
			//UE_LOG(LogTemp, Warning, TEXT("vpos: (%f, %f ): vuv: (%f, %f)"), VerticeIDs[i].Position.X, VerticeIDs[i].Position.Y, VerticeIDs[i].UV.X, VerticeIDs[i].UV.Y);
		}
		//UE_LOG(LogTemp, Warning, TEXT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"));

		// Create vertex buffer. Fill buffer with initial data upon creation
		FRHIResourceCreateInfo CreateInfo(TEXT("FRectangleVertexBuffer"),&Vertices);
		VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
	}

	UE::Math::TMatrix<float> GetMMatrix()
	{
		UE::Math::TMatrix<float> m = UE::Math::TMatrix<float>();
		UBVHData* BVHData = NewObject<UBVHData>();
		BVHData->Init(nullptr);
		BVHData->GetSceneData();

		if(BVHData->VerticeIDs.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("No Triangle Data"));
			return m;
		}
		
		TArray<AActor*> meshs = BVHData->GetMeshCollecter()->GetAllStaticMeshActors(false, true);
		AActor* mesh = meshs.Num() > 0? meshs[0] : nullptr;
		if(mesh != nullptr)
		{
			UE::Math::TMatrix<double> _m_Matrix = mesh->GetTransform().ToMatrixWithScale();
			UE::Math::TMatrix<float> M_Matrix = UE::Math::TMatrix<float>(_m_Matrix);
			m = M_Matrix;
		}

		return m;
	}

	virtual void ReleaseRHI() override
	{
		VertexBufferRHI.SafeRelease();
	}
};


/**
 * @brief 顶点序号buffer
 */
class FRectangleIndexBuffer : public FIndexBuffer
{
private:
	
public:

	int32 PrimitiveNum = 0;
	
	/** Initialize the RHI for this rendering resource */
	void InitRHI() override
	{
		UBVHData* BVHData = NewObject<UBVHData>();
		BVHData->Init(nullptr);
		BVHData->GetSceneData();

		
		//逆时针
		if(BVHData->TriangleVertexIDs.IsEmpty())
		{
			UE_LOG(LogTemp, Warning, TEXT("No Triangle Data"));
			return;
		}
		
		const int idNum = BVHData->TriangleVertexIDs.Num();
		uint16* Indices = new uint16[idNum];
		PrimitiveNum = idNum / 3;

		//UE_LOG(LogTemp, Warning, TEXT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"));
		for(int i = 0; i < idNum; i++)
		{
			uint16 u16ID = (uint16)(BVHData->TriangleVertexIDs[i]);
			Indices[i] = u16ID;
			//UE_LOG(LogTemp, Warning, TEXT("triID: %d"), Indices[i]);
		}
		//UE_LOG(LogTemp, Warning, TEXT("++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++"));

		//const uint16 Indices[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 7, 6, 9, 10, 11, 12, 13, 12, 11};

		TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
		//uint32 NumIndices = UE_ARRAY_COUNT(Indices);
		uint32 NumIndices = idNum;
		IndexBuffer.AddUninitialized(NumIndices);
		FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

		// Create index buffer. Fill buffer with initial data upon creation
		FRHIResourceCreateInfo CreateInfo(TEXT("FRectangleIndexBuffer"),&IndexBuffer);
		IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
	}

	virtual void ReleaseRHI() override
	{
		IndexBufferRHI.SafeRelease();
	}
};

class FTextureVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual void InitRHI() override
	{
		FVertexDeclarationElementList Elements;
		uint32 Stride = sizeof(FTextureVertex);
		if(Stride <= 0) return;

		//绑定cpu传入gpu的场景数据
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, Position), VET_Float2, 0, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, UV), VET_Float2, 1, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, Position_OS), VET_Float4, 2, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, Normal), VET_Float4, 3, Stride));
		Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, Tangent), VET_Float4, 4, Stride));
		VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
	}
	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};

/**
 * @brief globalshader
 */
class FRDGGlobalShader : public FGlobalShader
{
public:
	SHADER_USE_PARAMETER_STRUCT(FRDGGlobalShader, FGlobalShader);

	//添加要传入shader的参数
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER(FMatrix44f, M_Matrix)
		SHADER_PARAMETER(FMatrix44f, M_Matrix_Invers_Trans)
		RENDER_TARGET_BINDING_SLOTS()
	END_SHADER_PARAMETER_STRUCT()

	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters &Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::ES3_1);
	}
};

/**
 * @brief vertex shader
 */
class FSGVertexShader : public FRDGGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSGVertexShader);

	FSGVertexShader() {}

	FSGVertexShader(const ShaderMetaType::CompiledShaderInitializerType &Initializer) : FRDGGlobalShader(Initializer) {}
};


/**
 * @brief pixel shader
 */
class FSGPixelShader : public FRDGGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSGPixelShader);

	FSGPixelShader() {}

	FSGPixelShader(const ShaderMetaType::CompiledShaderInitializerType &Initializer) : FRDGGlobalShader(Initializer) {}
};

class FSGPixelPositionShader : public FRDGGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSGPixelPositionShader);

	FSGPixelPositionShader() {}

	FSGPixelPositionShader(const ShaderMetaType::CompiledShaderInitializerType &Initializer) : FRDGGlobalShader(Initializer) {}
};

class FSGPixelNormalShader : public FRDGGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSGPixelNormalShader);

	FSGPixelNormalShader() {}

	FSGPixelNormalShader(const ShaderMetaType::CompiledShaderInitializerType &Initializer) : FRDGGlobalShader(Initializer) {}
};

class FSGPixelTangentShader : public FRDGGlobalShader
{
public:
	DECLARE_GLOBAL_SHADER(FSGPixelTangentShader);

	FSGPixelTangentShader() {}

	FSGPixelTangentShader(const ShaderMetaType::CompiledShaderInitializerType &Initializer) : FRDGGlobalShader(Initializer) {}
};


/**
 * @brief compute shader
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

/*
 *  Vertex Resource Declaration
 */
extern TGlobalResource<FTextureVertexDeclaration> GTextureVertexDeclaration;
extern TGlobalResource<FRectangleVertexBuffer> GRectangleVertexBuffer;
extern TGlobalResource<FRectangleIndexBuffer> GRectangleIndexBuffer;

//RDG Method
void RDGCompute(FRHICommandListImmediate &RHIImmCmdList, FTexture2DRHIRef RenderTargetRHI);

void RDGDraw(FRHICommandListImmediate &RHIImmCmdList, FTexture2DRHIRef PositonRHI, FTexture2DRHIRef NormalRHI, FTexture2DRHIRef TangentRHI);

void DrawToRT(FRHICommandListImmediate &RHIImmCmdList, FTexture2DRHIRef RTRHI, OutRTType Type);
