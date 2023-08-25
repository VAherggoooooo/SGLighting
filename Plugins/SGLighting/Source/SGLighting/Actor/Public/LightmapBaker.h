// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGLighting/Rendering/SG_Data.h"
#include "Engine/DirectionalLight.h"
#include "GameFramework/Actor.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Components/LightComponent.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Engine/Classes/Materials/MaterialParameterCollection.h"
#include "SGLighting/Rendering/MakeLightmapRDG.h"
#include "LightmapBaker.generated.h"

#define SG_NUM 12

UCLASS()
class SGLIGHTING_API ALightmapBaker : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ALightmapBaker();
	virtual void OnConstruction(const FTransform& Transform) override;

	UPROPERTY(VisibleDefaultsOnly)
	USceneComponent* Root;

	
	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* SM_BlendMap;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* SM_CopyMap;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* SM_BlurMap;

	UPROPERTY(EditAnywhere)
	UStaticMeshComponent* SM_ExpandMap;


	UMaterialInstanceDynamic* Dy_Blend;
	UMaterialInstanceDynamic* Dy_Copy;
	UMaterialInstanceDynamic* Dy_Blur;
	UMaterialInstanceDynamic* Dy_Expand;


	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* Position;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* Normal;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* Tangent;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* RT1;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* RT2;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* RT3;

	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* RT4;

	UPROPERTY(EditAnywhere)
	ADirectionalLight* MainLight;

	UPROPERTY(EditAnywhere)
	int SampleCount = 3;
	UPROPERTY(EditAnywhere)
	int Depth = 2;

	UPROPERTY(EditAnywhere)
	float MaxFallOff = 3.2f;

	float Frame;

	UPROPERTY(EditAnywhere)
	int MaxBakeTime = 150;

	UPROPERTY(EditAnywhere)
	int BlurTimes = 4;

	UPROPERTY(EditAnywhere)
	float BlurIntensity = 2.4f;
	
	static TArray<FSG_Full> OutSGs;

	UPROPERTY(EditAnywhere)
	TArray<UTextureRenderTarget2D*> SGRTs;

	UPROPERTY(EditAnywhere)
	TArray<UTextureRenderTarget2D*> SGRTs_Temp;

	UPROPERTY(EditAnywhere)
	TArray<UTextureRenderTarget2D*> SGRTs_Cur;

	UPROPERTY(EditAnywhere)
	UTexture2D* AlbedoTex;

	UPROPERTY(EditAnywhere)
	float Roughness;

	UPROPERTY(EditAnywhere)
	UMaterialParameterCollection *Collection;
	
	UFUNCTION(BlueprintCallable)
	void ClearMap();


	void GetSamplerDirs();


	UFUNCTION(CallInEditor)
	void TestBake();

	void InitSGs();
	
	
	void PathTracingInLightmap(
		UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* Position_RT,
		UTextureRenderTarget2D* Normal_RT, UTextureRenderTarget2D* Tangent_RT,
		ADirectionalLight* mainLight, uint8 sampleCount, uint8 depth,
		float seed, UTextureRenderTarget2D* TestTexture,
		TArray<UTextureRenderTarget2D* >& OutSGRTs, float _MaxFallOff, float _Roughness);
protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	void BlendTexture(UTextureRenderTarget2D* PreTex, UTextureRenderTarget2D* CurTex, UTextureRenderTarget2D* OutTex);
	void BlurTexture(UTextureRenderTarget2D* CurTex, UTextureRenderTarget2D* TempTex, UTextureRenderTarget2D* OutTex);
	void ExpandTexture(UTextureRenderTarget2D* CurTex, UTextureRenderTarget2D* TempTex, UTextureRenderTarget2D* OutTex);
};




void ComputePathTracing(
	FRHICommandListImmediate &RHIImmCmdList,
	FTexture2DRHIRef RenderTargetRHI,
	FTexture2DRHIRef Position_RT,
	FTexture2DRHIRef Normal_RT,
	FTexture2DRHIRef Tangent_RT,
	ADirectionalLight* MainLight,
	uint8 SampleCount,
	uint8 depth,
	float seed,
	FTexture2DRHIRef TestTexture,
	TArray<FSG_Full>& OutSGs,
	TArray<FTexture2DRHIRef> OutSGRTs, FTexture2DRHIRef InAlbedoTex, float MaxFallOff, float _Roughness);
