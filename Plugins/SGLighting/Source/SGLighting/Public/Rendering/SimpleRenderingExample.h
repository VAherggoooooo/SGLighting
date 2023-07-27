#pragma once
#include "RenderGraph.h"

#include "CoreMinimal.h"
#include "UObject/ObjectMacros.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "SimpleRenderingExample.generated.h"

class FRHICommandListImmediate;
struct IPooledRenderTarget;

UCLASS(MinimalAPI, meta = (ScriptName = "SimpleRenderingExample"))
class USimpleRenderingExampleBlueprintLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()
	
	UFUNCTION(BlueprintCallable, Category = "SimpleRenderingExample", meta = (WorldContext = "WorldContextObject"))
	static void UseRDGComput(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderTarget);

	UFUNCTION(BlueprintCallable, Category = "SimpleRenderingExample", meta = (WorldContext = "WorldContextObject"))
	static void UseRDGDraw(const UObject* WorldContextObject, UTextureRenderTarget2D* OutputRenderTarget, UTexture2D* InTexture);
};

namespace SimpleRenderingExample
{
	/*
	 * 	Common Resource
	 */
	struct FTextureVertex
	{
		FVector4f Position;
		FVector2f UV;
	};

	class FRectangleVertexBuffer : public FVertexBuffer
	{
	public:
		/** Initialize the RHI for this rendering resource */
		void InitRHI() override
		{
			TResourceArray<FTextureVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
			Vertices.SetNumUninitialized(6);//一个四边形, 两个三角形, 六个点

			Vertices[0].Position = FVector4f(1, 1, 0, 1);//右下角
			Vertices[0].UV = FVector2f(1, 1);

			Vertices[1].Position = FVector4f(-1, 1, 0, 1);
			Vertices[1].UV = FVector2f(0, 1);

			Vertices[2].Position = FVector4f(1, -1, 0, 1);
			Vertices[2].UV = FVector2f(1, 0);

			Vertices[3].Position = FVector4f(-1, -1, 0, 1);//左上角
			Vertices[3].UV = FVector2f(0, 0);

			//The final two vertices are used for the triangle optimization (a single triangle spans the entire viewport )
			Vertices[4].Position = FVector4f(-1, 1, 0, 1);
			Vertices[4].UV = FVector2f(-1, 1);

			Vertices[5].Position = FVector4f(1, -1, 0, 1);
			Vertices[5].UV = FVector2f(1, -1);

			// Create vertex buffer. Fill buffer with initial data upon creation
			FRHIResourceCreateInfo CreateInfo(TEXT("FRectangleVertexBuffer"),&Vertices);
			VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
		}
	};

	class FRectangleIndexBuffer : public FIndexBuffer
	{
	public:
		/** Initialize the RHI for this rendering resource */
		void InitRHI() override
		{
			// Indices 0 - 5 are used for rendering a quad. Indices 6 - 8 are used for triangle optimization.
			const uint16 Indices[] = {0, 1, 2, 2, 1, 3, 0, 4, 5};

			TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
			uint32 NumIndices = UE_ARRAY_COUNT(Indices);
			IndexBuffer.AddUninitialized(NumIndices);
			FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

			// Create index buffer. Fill buffer with initial data upon creation
			FRHIResourceCreateInfo CreateInfo(TEXT("FRectangleIndexBuffer"),&IndexBuffer);
			IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
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
			Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, Position), VET_Float2, 0, Stride));
			Elements.Add(FVertexElement(0, STRUCT_OFFSET(FTextureVertex, UV), VET_Float2, 1, Stride));
			VertexDeclarationRHI = RHICreateVertexDeclaration(Elements);
		}
		virtual void ReleaseRHI() override
		{
			VertexDeclarationRHI.SafeRelease();
		}
	};

	/*
	 *  Vertex Resource Declaration
	 */
	extern TGlobalResource<FTextureVertexDeclaration> GTextureVertexDeclaration;
	extern TGlobalResource<FRectangleVertexBuffer> GRectangleVertexBuffer;
	extern TGlobalResource<FRectangleIndexBuffer> GRectangleIndexBuffer;

	//RDG Method
	void RDGCompute(FRHICommandListImmediate &RHIImmCmdList, FTexture2DRHIRef RenderTargetRHI, FSimpleShaderParameter InParameter);

	void RDGDraw(FRHICommandListImmediate &RHIImmCmdList, FTexture2DRHIRef RenderTargetRHI, FSimpleShaderParameter InParameter, FTexture2DRHIRef InTexture);
} // namespace SimpleRenderingExample
