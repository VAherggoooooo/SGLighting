// Fill out your copyright notice in the Description page of Project Settings.


#include "SGLighting/Actor/Public/LightmapBaker.h"
#include "SGLighting/Rendering/SG_Data.h"
#include "Components/LightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMaterialLibrary.h"
#include "Kismet/KismetRenderingLibrary.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Materials/MaterialParameterCollection.h"
#include "SGLighting/Rendering/MakeLightmapRDG.h"
#include "SGLighting/Rendering/SGPathTracing.h"
#include "UObject/ConstructorHelpers.h"


TArray<FSG_Full> ALightmapBaker::OutSGs;

// Sets default values
ALightmapBaker::ALightmapBaker()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;
	
	SM_BlendMap = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlendMap"));
	SM_CopyMap = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CopyMap"));
	SM_BlurMap = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BlurMap"));
	SM_ExpandMap = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("ExpandMap"));

	FSoftObjectPath PlaneMesh_SoftRef("StaticMesh'/Engine/BasicShapes/Plane.Plane'");
	SM_BlendMap->SetStaticMesh(Cast<UStaticMesh>(PlaneMesh_SoftRef.TryLoad()));
	SM_CopyMap->SetStaticMesh(Cast<UStaticMesh>(PlaneMesh_SoftRef.TryLoad()));
	SM_BlurMap->SetStaticMesh(Cast<UStaticMesh>(PlaneMesh_SoftRef.TryLoad()));
	SM_ExpandMap->SetStaticMesh(Cast<UStaticMesh>(PlaneMesh_SoftRef.TryLoad()));

	//为什么不用for循环? 因为我试了就是不能
	FSoftObjectPath BlendMat_SoftRef("MaterialInstanceConstant'/SGLighting/Material/EditTexture/BlendMap_Inst.BlendMap_Inst'");
	SM_BlendMap->SetMaterial(0, Cast<UMaterialInstanceConstant>(BlendMat_SoftRef.TryLoad()));
	FSoftObjectPath CopyMat_SoftRef("MaterialInstanceConstant'/SGLighting/Material/EditTexture/CopyTexture_Inst.CopyTexture_Inst'");
	SM_CopyMap->SetMaterial(0, Cast<UMaterialInstanceConstant>(CopyMat_SoftRef.TryLoad()));
	FSoftObjectPath BlurMat_SoftRef("MaterialInstanceConstant'/SGLighting/Material/EditTexture/BlurTexture_Inst.BlurTexture_Inst'");
	SM_BlurMap->SetMaterial(0, Cast<UMaterialInstanceConstant>(BlurMat_SoftRef.TryLoad()));
	FSoftObjectPath ExpandMat_SoftRef("MaterialInstanceConstant'/SGLighting/Material/EditTexture/ExpandTexture_Inst.ExpandTexture_Inst'");
	SM_ExpandMap->SetMaterial(0, Cast<UMaterialInstanceConstant>(ExpandMat_SoftRef.TryLoad()));

	
	SM_BlendMap->SetVisibility(false);
	SM_CopyMap->SetVisibility(false);
	SM_BlurMap->SetVisibility(false);
	SM_ExpandMap->SetVisibility(false);
	
	SM_BlendMap->SetupAttachment(Root);
	SM_CopyMap->SetupAttachment(Root);
	SM_BlurMap->SetupAttachment(Root);
	SM_ExpandMap->SetupAttachment(Root);

	
	//为什么不用for循环? 因为我试了就是不能
	FSoftObjectPath PositinRT_SoftRef("TextureRenderTarget2D'/SGLighting/Textures/DataRTs/PositionRT.PositionRT'");
	Position_RT = Cast<UTextureRenderTarget2D>(PositinRT_SoftRef.TryLoad());

	FSoftObjectPath NormalRT_SoftRef("TextureRenderTarget2D'/SGLighting/Textures/DataRTs/NormalRT.NormalRT'");
	Normal_RT = Cast<UTextureRenderTarget2D>(NormalRT_SoftRef.TryLoad());

	FSoftObjectPath TangentRT_SoftRef("TextureRenderTarget2D'/SGLighting/Textures/DataRTs/TangentRT.TangentRT'");
	Tangent_RT = Cast<UTextureRenderTarget2D>(TangentRT_SoftRef.TryLoad());

	FSoftObjectPath PT1RT_SoftRef("TextureRenderTarget2D'/SGLighting/Textures/PT_RT/PR_RT1.PR_RT1'");
	PT_RT1 = Cast<UTextureRenderTarget2D>(PT1RT_SoftRef.TryLoad());
	
	FSoftObjectPath PT2RT_SoftRef("TextureRenderTarget2D'/SGLighting/Textures/PT_RT/PT_RT2.PT_RT2'");
	PT_RT2 = Cast<UTextureRenderTarget2D>(PT2RT_SoftRef.TryLoad());
	
	FSoftObjectPath PT3RT_SoftRef("TextureRenderTarget2D'/SGLighting/Textures/PT_RT/PT_RT3.PT_RT3'");
	PT_RT3= Cast<UTextureRenderTarget2D>(PT3RT_SoftRef.TryLoad());


	
	SGRTs.SetNum(SG_NUM);
	SGRTs_Cur.SetNum(SG_NUM);
	SGRTs_Temp.SetNum(SG_NUM);

	for(int i = 0; i < SG_NUM; i++)
	{
		FSoftObjectPath SG_SoftRef;
		FString _path = FString::Format(TEXT("TextureRenderTarget2D'/SGLighting/Textures/SGRTs/OutSG/SG{0}.SG{0}'"), { i+ 1 });
		SG_SoftRef.SetPath(_path);
		SGRTs[i]= Cast<UTextureRenderTarget2D>(SG_SoftRef.TryLoad());

		_path = FString::Format(TEXT("TextureRenderTarget2D'/SGLighting/Textures/SGRTs/TempSG1/SG{0}_2.SG{0}_2'"), { i+ 1 });
		SG_SoftRef.SetPath(_path);
		SGRTs_Cur[i]= Cast<UTextureRenderTarget2D>(SG_SoftRef.TryLoad());

		_path = FString::Format(TEXT("TextureRenderTarget2D'/SGLighting/Textures/SGRTs/TempSG2/SG{0}_2.SG{0}_2'"), { i+ 1 });
		SG_SoftRef.SetPath(_path);
		SGRTs_Temp[i]= Cast<UTextureRenderTarget2D>(SG_SoftRef.TryLoad());
	}
}


void ALightmapBaker::OnConstruction(const FTransform& Transform)
{
	Super::OnConstruction(Transform);
}

void ALightmapBaker::FastBake()
{
	InitSGs();
	
	UseRDGDraw(nullptr, Position_RT, Normal_RT, Tangent_RT);
	MainLight = GetMainLight(nullptr);
	if(MainLight == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("LOST MAINLIGHT!"));
		return;
	}
	PathTracingInLightmap(PT_RT1, Position_RT, Normal_RT, Tangent_RT, MainLight, SampleCount, Depth, Frame, SGRTs, MaxFallOff, Roughness);
}

void ALightmapBaker::ClearBakeMap()
{
	for(int i = 0; i < SG_NUM; i++)
	{
		UKismetRenderingLibrary::ClearRenderTarget2D(this, SGRTs[i]);
		UKismetRenderingLibrary::ClearRenderTarget2D(this, SGRTs_Cur[i]);
		UKismetRenderingLibrary::ClearRenderTarget2D(this, SGRTs_Temp[i]);
	}
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



ADirectionalLight* ALightmapBaker::GetMainLight(ADirectionalLight* _MainLight)
{
	if(_MainLight == nullptr)
	{
		TArray<AActor*> Actors;
		UGameplayStatics::GetAllActorsOfClass(GetWorld(), ADirectionalLight::StaticClass(), Actors);
		if(Actors.Num() <= 0)
		{
			UE_LOG(LogTemp, Error, TEXT("No Main Light!"));
		}
		else
		{
			_MainLight = static_cast<ADirectionalLight*>(Actors[0]);
		}
	}
	if(_MainLight == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("NULL MAINLIGHT!"));
	}

	return _MainLight;
}

void ComputePathTracing(
	FRHICommandListImmediate &RHIImmCmdList,
	FTexture2DRHIRef RenderTargetRHI,
	FTexture2DRHIRef Position_RT,
	FTexture2DRHIRef Normal_RT,
	FTexture2DRHIRef Tangent_RT,
	ADirectionalLight* _MainLight,
	uint8 SampleCount, uint8 depth,
	float seed,
	TArray<FSG_Full>& SGs,
	TArray<FTexture2DRHIRef> OutSGRTs, FTexture2DRHIRef InAlbedoTex, float MaxFallOff, float _Roughness)
{
	check(IsInRenderingThread());
	
	FRDGBuilder GraphBuilder(RHIImmCmdList);

	
	//Create RenderTargetDesc
	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget;
	TRefCountPtr<IPooledRenderTarget> PooledRenderTarget_Test;
	
	//RDG Begin
	FSGComputeShader_PT::FParameters *Parameters = GraphBuilder.AllocParameters<FSGComputeShader_PT::FParameters>();
	
	const FRDGTextureDesc& RenderTargetDesc = FRDGTextureDesc::Create2D(RenderTargetRHI->GetSizeXY(),RenderTargetRHI->GetFormat(), FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV);
	FRDGTextureRef RDGRenderTarget = GraphBuilder.CreateTexture(RenderTargetDesc, TEXT("RDGRenderTarget"));
	Parameters->OutTexture = GraphBuilder.CreateUAV(FRDGTextureUAVDesc(RDGRenderTarget));
	
	//把SG RT传入shader
	TArray<FRDGTextureDesc> SGDesc;
	TArray<FRDGTextureRef> SGRefs;
	TArray<TRefCountPtr<IPooledRenderTarget>> PooledSG;
	SGDesc.SetNum(SG_NUM);
	SGRefs.SetNum(SG_NUM);
	PooledSG.SetNum(SG_NUM);
	for(int i = 0; i < SG_NUM; i++)
	{
		SGDesc[i] = (FRDGTextureDesc::Create2D(Position_RT->GetSizeXY(),Position_RT->GetFormat(), FClearValueBinding::Black, TexCreate_RenderTargetable | TexCreate_ShaderResource | TexCreate_UAV));
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
	Parameters->Albedo = InAlbedoTex;
	Parameters->AlbedoSampler = TStaticSamplerState<SF_Trilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	
	Parameters->InPosition = Position_RT;
	Parameters->InPositionSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	Parameters->InNormal = Normal_RT;
	Parameters->InNormalSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	Parameters->InTangent = Tangent_RT;
	Parameters->InTangentSampler = TStaticSamplerState<SF_Point, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI();
	
	FRDGBufferRef TriBuffer = CreateStructuredBuffer(GraphBuilder, TEXT("TriangleDataBuffer"), GRectangleVertexBuffer.SceneMeshTriangles,ERDGInitialDataFlags::NoCopy);//GRectangleVertexBuffer.MeshTriangles
	Parameters->TriangleBuffer = GraphBuilder.CreateSRV(TriBuffer);
	Parameters->TriangleNum = GRectangleVertexBuffer.SceneMeshTriangles.Num();

	Parameters->SampleCount = SampleCount;
	Parameters->depth = depth;
	Parameters->seed = seed;
	Parameters->Roughness = _Roughness;

	if(_MainLight == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("NULL MAINLIGHT!"));
		return;
	}
	TArray<FMainLight> MainLights;
	FMainLight _L = FMainLight(FVector3f(_MainLight->GetActorRotation().Vector()),FVector3f(_MainLight->GetLightColor()), _MainLight->GetLightComponent()->Intensity);
	MainLights.Add(_L);
	FRDGBufferRef MainLightBuff = CreateStructuredBuffer(GraphBuilder, TEXT("MainLightDataBuffer"), MainLights,ERDGInitialDataFlags::NoCopy);//GRectangleVertexBuffer.MeshTriangles
	Parameters->MainLightBuffer = GraphBuilder.CreateSRV(MainLightBuff);
	Parameters->MaxFallOff = MaxFallOff;
	TArray<FSG> FSGs;
	for(FSG_Full sgf : SGs)
	{
		FSGs.Add(FSG(sgf.Axis, sgf.Amplitude, sgf.Sharpness, sgf.BasisSqIntegralOverDomain));
		
	}
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
	for(int i = 0; i < SG_NUM; i++)
	{
		GraphBuilder.QueueTextureExtraction(SGRefs[i] , &(PooledSG[i]));
	}
	GraphBuilder.Execute();
	
	//Copy Result To RenderTarget Asset
	RHIImmCmdList.CopyTexture(PooledRenderTarget->GetRenderTargetItem().ShaderResourceTexture, RenderTargetRHI->GetTexture2D(), FRHICopyTextureInfo());

	for(int i = 0; i < SG_NUM; i++)
	{
		RHIImmCmdList.CopyTexture(PooledSG[i]->GetRenderTargetItem().ShaderResourceTexture, OutSGRTs[i]->GetTexture2D(), FRHICopyTextureInfo());
	}
}



void ALightmapBaker::PathTracingInLightmap(UTextureRenderTarget2D* OutputRT,
                                           UTextureRenderTarget2D* _Position_RT, UTextureRenderTarget2D* _Normal_RT, UTextureRenderTarget2D* _Tangent_RT,
                                           ADirectionalLight* mainLight, uint8 sampleCount, uint8 depth,
                                           float seed, 
                                           TArray<UTextureRenderTarget2D* >& OutSGRTs, float _MaxFallOff, float _Roughness)
{
	check(IsInGameThread());

	if(Collection)
	{
		UKismetMaterialLibrary::SetScalarParameterValue(this, Collection, FName("Roughness"), Roughness);
	}

	FTexture2DRHIRef RenderTargetRHI = OutputRT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef positionRT = _Position_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef normalRT = _Normal_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	FTexture2DRHIRef tangentRT = _Tangent_RT->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();

	FTexture2DRHIRef InAlbedoTex = AlbedoTex->GetResource()->TextureRHI->GetTexture2D();
	


	TArray<FTexture2DRHIRef> SGRT_Ref;
	SGRT_Ref.SetNum(SG_NUM);
	for(int i = 0; i < SG_NUM; i++)
	{
		SGRT_Ref[i] = OutSGRTs[i]->GameThread_GetRenderTargetResource()->GetRenderTargetTexture();
	}
	if(mainLight == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("NULL MAINLIGHT!"));
		return;
	}
	ENQUEUE_RENDER_COMMAND(CaptureCommand)
	(
		[RenderTargetRHI,
			positionRT, normalRT, tangentRT,
			mainLight,
			sampleCount, depth, seed,
			SGRT_Ref, InAlbedoTex, _MaxFallOff, _Roughness](FRHICommandListImmediate &RHICmdList)
		{
			ComputePathTracing(
				RHICmdList, RenderTargetRHI,
				positionRT, normalRT, tangentRT,
				mainLight, sampleCount, depth, seed, ALightmapBaker::OutSGs, SGRT_Ref, InAlbedoTex, _MaxFallOff, _Roughness);
		}
	);
}

void ALightmapBaker::UseRDGDraw(const UObject* WorldContextObject, UTextureRenderTarget2D* Output_Position_RT, UTextureRenderTarget2D* Output_Normal_RT, UTextureRenderTarget2D* Output_Tangent_RT)
{
	check(IsInGameThread());
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



// Called when the game starts or when spawned
void ALightmapBaker::BeginPlay()
{
	Super::BeginPlay();
	InitSGs();
	
	Dy_Blend = SM_BlendMap->CreateDynamicMaterialInstance(0, SM_BlendMap->GetMaterial(0));
	Dy_Copy = SM_CopyMap->CreateDynamicMaterialInstance(0, SM_CopyMap->GetMaterial(0));
	Dy_Blur = SM_BlurMap->CreateDynamicMaterialInstance(0, SM_BlurMap->GetMaterial(0));
	Dy_Expand = SM_ExpandMap->CreateDynamicMaterialInstance(0, SM_ExpandMap->GetMaterial(0));

	// for(int i = 0; i < SG_NUM; i++)
	// {
	// 	SGRTs_Temp.Add(UKismetRenderingLibrary::CreateRenderTarget2D(this, 1024, 1024));
	// 	SGRTs_Cur.Add(UKismetRenderingLibrary::CreateRenderTarget2D(this, 1024, 1024));
	// }
	

	UseRDGDraw(nullptr, Position_RT, Normal_RT, Tangent_RT);
}

// Called every frame
void ALightmapBaker::Tick(float DeltaTime)
{
	if(!bEnableTickBake) return;
	
	Super::Tick(DeltaTime);

	if(Frame < MaxBakeTime)
	{
		GetMainLight(MainLight);
		PathTracingInLightmap(PT_RT1, Position_RT, Normal_RT, Tangent_RT, MainLight, SampleCount, Depth, Frame, SGRTs_Cur, MaxFallOff, Roughness);

		Dy_Blend->SetScalarParameterValue(FName("Fram"), Frame);
		
		BlendTexture(PT_RT2, PT_RT1, PT_RT3);

		for(int i = 0; i < SG_NUM; i++)
		{
			BlendTexture(SGRTs_Temp[i], SGRTs_Cur[i], SGRTs[i]);
		}
	
		Frame += DeltaTime;

		if(Frame >= MaxBakeTime)
		{
			for(int i = 0; i < BlurTimes; i++)
			{
				for(int j = 0; j < SG_NUM; j++)
				{
					BlurTexture(SGRTs[j], SGRTs_Temp[j], SGRTs[j]);
				}
			}
			if(bEnableExpand)
			{
				for(int j = 0; j < SG_NUM; j++)
				{
					ExpandTexture(SGRTs[j], SGRTs_Temp[j], SGRTs[j]);
				}
			}
			UKismetSystemLibrary::PrintString(this, FString("Bake Over"));
		}
	}
}

void ALightmapBaker::BlendTexture(UTextureRenderTarget2D* PreTex, UTextureRenderTarget2D* CurTex,
	UTextureRenderTarget2D* OutTex)
{
	if(Dy_Blend == nullptr || Dy_Copy == nullptr) return;

	//blend texture
	Dy_Blend->SetTextureParameterValue(FName("PreMap"), PreTex);
	Dy_Blend->SetTextureParameterValue(FName("CurMap"), CurTex);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, OutTex, Dy_Blend);

	//copy texture
	Dy_Copy->SetTextureParameterValue(FName("OriTex"), OutTex);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, PreTex, Dy_Copy);
}

void ALightmapBaker::BlurTexture(UTextureRenderTarget2D* CurTex, UTextureRenderTarget2D* TempTex,
	UTextureRenderTarget2D* OutTex)
{
	//模糊curtex输出到temptex, 再拷贝到outtex
	Dy_Blur->SetTextureParameterValue(FName("CurTex"), CurTex);
	Dy_Blur->SetScalarParameterValue(FName("BlurIntensity"), BlurIntensity);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, TempTex, Dy_Blur);

	//copy texture
	Dy_Copy->SetTextureParameterValue(FName("OriTex"), TempTex);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, OutTex, Dy_Copy);
	
}

void ALightmapBaker::ExpandTexture(UTextureRenderTarget2D* CurTex, UTextureRenderTarget2D* TempTex,
	UTextureRenderTarget2D* OutTex)
{
	Dy_Expand->SetTextureParameterValue(FName("InTex"), CurTex);

	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, TempTex, Dy_Expand);

	//copy texture
	Dy_Copy->SetTextureParameterValue(FName("OriTex"), TempTex);
	UKismetRenderingLibrary::DrawMaterialToRenderTarget(this, OutTex, Dy_Copy);
}
