// Fill out your copyright notice in the Description page of Project Settings.


#include "SGLighting/Actor/Public/LightmapBaker.h"

#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "SGLighting/Rendering/MakeLightmapRDG.h"
#include "SGLighting/Rendering/SGPathTracing.h"


TArray<FSG_Full> ALightmapBaker::OutSGs;

// Sets default values
ALightmapBaker::ALightmapBaker()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;


	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	
	SM_BlurMap = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlurMap"));
	SM_CopyMap = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CopyMap"));

	SM_BlurMap->SetupAttachment(Root);
	SM_CopyMap->SetupAttachment(Root);
}

void ALightmapBaker::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
	
}

void ALightmapBaker::ClearMap()
{
	UKismetRenderingLibrary::ClearRenderTarget2D(this, RT1);
	UKismetRenderingLibrary::ClearRenderTarget2D(this, RT2);
	UKismetRenderingLibrary::ClearRenderTarget2D(this, RT3);
}

void ALightmapBaker::GetSamplerDirs()
{
	
}

void ALightmapBaker::TestBake()
{
	InitSGs();
	UMakeLightmapBlueprintLibrary::UseRDGDraw(nullptr, Position, Normal, Tangent);
	PathTracingInLightmap(RT1, Position, Normal, Tangent, MainLight, SampleCount, Depth, Frame, RT4, SGRTs);
}

void ALightmapBaker::InitSGs()
{
	OutSGs.SetNum(SG_NUM);
	int i = 0;
	for(UTextureRenderTarget2D* rt : SGRTs)
	{
		OutSGs[i].OutputRT =  rt;
		i++;
	}

	GenerateUniformSGs(SG_NUM, OutSGs);
}

void ComputePathTracing(
	FRHICommandListImmediate &RHIImmCmdList,
	FTexture2DRHIRef RenderTargetRHI,
	FTexture2DRHIRef Position_RT,
	FTexture2DRHIRef Normal_RT,
	FTexture2DRHIRef Tangent_RT,
	ADirectionalLight* MainLight,
	uint8 SampleCount, uint8 depth,
	float seed,
	FTexture2DRHIRef TestTexture,
	TArray<FSG_Full>& SGs,
	TArray<FTexture2DRHIRef> OutSGRTs)
{
	check(IsInRenderingThread());

	//TArray<FSG_Full> SGs;
	//GenerateUniformSGs(12, SGs);//init and get sg lobe
	//UE_LOG(LogTemp, Warning, TEXT("RT: %s"), *(OutSGRTs[0]->GetName()));
	
	FRDGBuilder GraphBuilder(RHIImmCmdList);

	
	//Create RenderTargetDesc
	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;
	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget_Test;
	
	int dirNum = RenderTargetRHI->GetSizeXY().X * RenderTargetRHI->GetSizeXY().Y * SampleCount;
	//RDG Begin
	FSGComputeShader_PT::FParameters *Parameters = GraphBuilder.AllocParameters<FSGComputeShader_PT::FParameters>();
	
	const FRDGTextureDesc& RenderTargetDesc = FRDGTextureDesc::Create2D(RenderTargetRHI->GetSizeXY(),RenderTargetRHI->GetFormat(), FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef RDGRenderTarget = GraphBuilder.CreateTexture(RenderTargetDesc, TEXT("RDGRenderTarget"));
	Parameters->OutTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(RDGRenderTarget));

	const FRDGTextureDesc& RenderTargetDesc_Test = FRDGTextureDesc::Create2D(TestTexture->GetSizeXY(),TestTexture->GetFormat(), FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef RDGRenderTarget_Test = GraphBuilder.CreateTexture(RenderTargetDesc_Test, TEXT("RDGRenderTargetTEST"));
	Parameters->TestTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(RDGRenderTarget_Test));

	//TODO: 把SG RT传入shader
	//
	//
	TArray<FRDGTextureDesc> SGDesc;
	TArray<FRDGTextureRef> SGRefs;
	TArray<TRefCountPtr<IPooledRenderTarget>> PooledSG;
	SGDesc.SetNum(SG_NUM);
	SGRefs.SetNum(SG_NUM);
	PooledSG.SetNum(SG_NUM);
	for(int i = 0; i < SG_NUM; i++)
	{
		SGDesc[i] = (FRDGTextureDesc::Create2D(TestTexture->GetSizeXY(),TestTexture->GetFormat(), FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV));
		SGRefs[i] = GraphBuilder.CreateTexture((SGDesc[i]), TEXT("SGRT"));
		FRDGTextureUAVRef tref = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(SGRefs[i]));
 
		switch (i)
		{
		case 0:
			Parameters->SG1 = tref;
			break;
		case 1:
			Parameters->SG2 = tref;
			break;
		case 2:
			Parameters->SG3 = tref;
			break;
		case 3:
			Parameters->SG4 = tref;
			break;
		case 4:
			Parameters->SG5 = tref;
			break;
		case 5:
			Parameters->SG6 = tref;
			break;
		case 6:
			Parameters->SG7 = tref;
			break;
		case 7:
			Parameters->SG8 = tref;
			break;
		case 8:
			Parameters->SG9 = tref;
			break;
		case 9:
			Parameters->SG10 = tref;
			break;
		case 10:
			Parameters->SG11 = tref;
            break;
		case 11:
			Parameters->SG12 = tref;
			break;
		}
		
	}
	
	Parameters->InPosition = Position_RT;
	Parameters->InPositionSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	Parameters->InNormal = Normal_RT;
	Parameters->InNormalSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	Parameters->InTangent = Tangent_RT;
	Parameters->InTangentSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	
	FRDGBufferRef TriBuffer = CreateStructuredBuffer(GraphBuilder, TEXT("TriangleDataBuffer"), GRectangleVertexBuffer.SceneMeshTriangles,ERDGInitialDataFlags::NoCopy);//GRectangleVertexBuffer.MeshTriangles
	Parameters->TriangleBuffer = GraphBuilder.CreateSRV(TriBuffer);
	Parameters->TriangleNum = GRectangleVertexBuffer.SceneMeshTriangles.Num();

	Parameters->SampleCount = SampleCount;
	Parameters->depth = depth;
	Parameters->seed = seed;
	
	TArray<FMainLight> MainLights;
	FMainLight _L = FMainLight(FVector3f(MainLight->GetActorRotation().Vector()),FVector3f(MainLight->GetLightColor()), MainLight->GetLightComponent()->Intensity);
	MainLights.Add(_L);
	FRDGBufferRef MainLightBuff = CreateStructuredBuffer(GraphBuilder, TEXT("MainLightDataBuffer"), MainLights,ERDGInitialDataFlags::NoCopy);//GRectangleVertexBuffer.MeshTriangles
	Parameters->MainLightBuffer = GraphBuilder.CreateSRV(MainLightBuff);

	TArray<FSG> FSGs;
	for(FSG_Full sgf : SGs)
	{
		FSGs.Add(FSG(sgf.Axis, sgf.Amplitude, sgf.Sharpness, sgf.BasisSqIntegralOverDomain));
		
	}
	// for(int i = 0; i < SG_NUM; i++)
	// {
	// 	UE_LOG(LogTemp, Warning, TEXT("DIR: %s"), *(FSGs[i].Axis.ToString()));
	// }
	FRDGBufferRef sgbuf = CreateStructuredBuffer(GraphBuilder, TEXT("SGBuffer"), FSGs,ERDGInitialDataFlags::NoCopy);//GRectangleVertexBuffer.MeshTriangles
	Parameters->SGBuffer = GraphBuilder.CreateSRV(sgbuf);
	
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
	GraphBuilder.QueueTextureExtraction(RDGRenderTarget_Test, &PooledRenderTarget_Test);
	for(int i = 0; i < SG_NUM; i++)
	{
		GraphBuilder.QueueTextureExtraction(SGRefs[i] , &(PooledSG[i]));
	}
	GraphBuilder.Execute();
	
	//Copy Result To RenderTarget Asset
	RHIImmCmdList.CopyTexture(PooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, RenderTargetRHI->GetTexture2D(), FRHICopyTextureInfo());
	RHIImmCmdList.CopyTexture(PooledRenderTarget_Test->GetRenderTargetItem().ShaderResourceTexture, TestTexture->GetTexture2D(), FRHICopyTextureInfo());

	for(int i = 0; i < SG_NUM; i++)
	{
		RHIImmCmdList.CopyTexture(PooledSG[i]->GetRenderTargetItem().ShaderResourceTexture, OutSGRTs[i]->GetTexture2D(), FRHICopyTextureInfo());
	}
}



void ALightmapBaker::PathTracingInLightmap(UTextureRenderTarget2D* OutputRT,
                                           UTextureRenderTarget2D* Position_RT, UTextureRenderTarget2D* Normal_RT, UTextureRenderTarget2D* Tangent_RT,
                                           ADirectionalLight* mainLight, uint8 sampleCount, uint8 depth,
                                           float seed, UTextureRenderTarget2D* TestTexture,
                                           TArray<UTextureRenderTarget2D* >& OutSGRTs)
{
	check(IsInGameThread());

	FTexture2DRHIRef RenderTargetRHI = OutputRT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef positionRT = Position_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef normalRT = Normal_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef tangentRT = Tangent_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();

	
	
	FTexture2DRHIRef testTextureRT = TestTexture->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();


	TArray<FTexture2DRHIRef> SGRT_Ref;
	SGRT_Ref.SetNum(SG_NUM);
	for(int i = 0; i < SG_NUM; i++)
	{
		SGRT_Ref[i] = OutSGRTs[i]->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	}
	
	ENQUEUE_RENDER_COMMAND(CaptureCommand)
	(
		[RenderTargetRHI, positionRT, normalRT, tangentRT, mainLight, sampleCount, depth, seed, testTextureRT, SGRT_Ref](FRHICommandListImmediate &RHICmdList)
		{
			ComputePathTracing(RHICmdList, RenderTargetRHI, positionRT, normalRT, tangentRT, mainLight, sampleCount, depth, seed, testTextureRT, ALightmapBaker::OutSGs, SGRT_Ref);
		}
	);
}




// Called when the game starts or when spawned
void ALightmapBaker::BeginPlay()
{
	Super::BeginPlay();
	InitSGs();
	
	Dy_Blur = SM_BlurMap->CreateDynamicMaterialInstance(0, SM_BlurMap->GetMaterial(0));
	Dy_Copy = SM_CopyMap->CreateDynamicMaterialInstance(0, SM_CopyMap->GetMaterial(0));

	// for(int i = 0; i < SG_NUM; i++)
	// {
	// 	SGRTs_Temp.Add(UKismetRenderingLibrary::CreateRenderTarget2D(this, 1024, 1024));
	// 	SGRTs_Cur.Add(UKismetRenderingLibrary::CreateRenderTarget2D(this, 1024, 1024));
	// }
	

	UMakeLightmapBlueprintLibrary::UseRDGDraw(nullptr, Position, Normal, Tangent);
}

// Called every frame
void ALightmapBaker::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if(Frame < MaxBakeTime)
	{
		//USGPathTracing::PathTracingInLightmap(nullptr, RT1, Position, Normal, Tangent, MainLight, SampleCount, Depth, Frame, RT4);
		//PathTracingInLightmap(RT1, Position, Normal, Tangent, MainLight, SampleCount, Depth, Frame, RT4, SGRTs);
		PathTracingInLightmap(RT1, Position, Normal, Tangent, MainLight, SampleCount, Depth, Frame, RT4, SGRTs_Cur);

		Dy_Blur->SetScalarParameterValue(FName("Fram"), Frame);
		
		// Dy_Blur->SetTextureParameterValue(FName("PreMap"), RT2);
		// Dy_Blur->SetTextureParameterValue(FName("CurMap"), RT1);
		// UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT3, Dy_Blur);

		// Dy_Copy->SetTextureParameterValue(FName("OriTex"), RT3);
		// UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, RT2, Dy_Copy);

		BlendTexture(RT2, RT1, RT3);

		for(int i = 0; i < SG_NUM; i++)
		{
			BlendTexture(SGRTs_Temp[i], SGRTs_Cur[i], SGRTs[i]);
		}
	
		Frame += DeltaTime;

		if(Frame >= MaxBakeTime)
		{
			UKismetSystemLibrary::PrintString(this, FString("Bake Over"));
		}
	}
}

void ALightmapBaker::BlendTexture(UTextureRenderTarget2D* PreTex, UTextureRenderTarget2D* CurTex,
	UTextureRenderTarget2D* OutTex)
{
	if(Dy_Blur == nullptr || Dy_Copy == nullptr) return;
	Dy_Blur->SetTextureParameterValue(FName("PreMap"), PreTex);
	Dy_Blur->SetTextureParameterValue(FName("CurMap"), CurTex);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, OutTex, Dy_Blur);

	Dy_Copy->SetTextureParameterValue(FName("OriTex"), OutTex);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, PreTex, Dy_Copy);
}

