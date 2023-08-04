#pragma once


class FMyShaderVertexBuffer : public FVertexBuffer
{
public:
	int32 VertexNum = 0;
	
	virtual void InitRHI() override;

	virtual void ReleaseRHI() override
	{
		VertexBufferRHI.SafeRelease();
	}
};



class FMyShaderIndexBuffer : public FIndexBuffer
{
public:
	int32 PrimitiveNum = 0;

	virtual void InitRHI() override;

	virtual void ReleaseRHI() override
	{
		IndexBufferRHI.SafeRelease();
	}
};



class FVertexDeclarationRes : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	
	virtual void InitRHI() override;
	virtual void ReleaseRHI() override
	{
		VertexDeclarationRHI.SafeRelease();
	}
};