// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "SGLighting/Rendering/SG_Data.h"
#include "Engine/DirectionalLight.h"
#include "GameFramework/Actor.h"
#include "RenderGraphBuilder.h"
#include "RenderGraphUtils.h"
#include "Engine/TextureRenderTarget2D.h"
#include "LightmapBaker.generated.h"




UCLASS()
class SGLIGHTING_API ALightmapBaker : public AActor
{
	GENERATED_BODY()

public:
	ALightmapBaker();
	virtual void OnConstruction(const FTransform& Transform) override;

	/**
	 *  是否使用多帧烘焙?
	 */
	UPROPERTY(EditAnywhere)
	bool bEnableTickBake = true;
	
	UPROPERTY(VisibleDefaultsOnly)
	USceneComponent* Root;

	/**
	 *  记录bake point position
	 */
	UPROPERTY(VisibleDefaultsOnly)
	UTextureRenderTarget2D* Position_RT;

	/**
	 *  记录bake point normal
	 */
	UPROPERTY(VisibleDefaultsOnly)
	UTextureRenderTarget2D* Normal_RT;

	/**
	 *  记录bake point tangent (暂时没啥用)
	 */
	UPROPERTY(VisibleDefaultsOnly)
	UTextureRenderTarget2D* Tangent_RT;

	/**
	 *  用于创建动态材质实例, 混合贴图
	 */
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* SM_BlendMap;

	/**
	 *  用于创建动态材质实例, 拷贝贴图
	 */
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* SM_CopyMap;

	/**
	 *  用于创建动态材质实例, 模糊贴图
	 */
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* SM_BlurMap;

	/**
	 *  用于创建动态材质实例, 按像素扩张贴图 (算法待优化)
	 */
	UPROPERTY(VisibleDefaultsOnly)
	UStaticMeshComponent* SM_ExpandMap;

	
	UPROPERTY()
	UMaterialInstanceDynamic* Dy_Blend;
	UPROPERTY()
	UMaterialInstanceDynamic* Dy_Copy;
	UPROPERTY()
	UMaterialInstanceDynamic* Dy_Blur;
	UPROPERTY()
	UMaterialInstanceDynamic* Dy_Expand;

	/**
	 * 用于记录path tracing结果
	 */
	UPROPERTY(VisibleDefaultsOnly)
	UTextureRenderTarget2D* PT_RT1;

	/**
	 *  用于记录path tracing结果 (临时中间贴图)
	 */
	UPROPERTY(VisibleDefaultsOnly)
	UTextureRenderTarget2D* PT_RT2;

	/**
	 *  用于记录path tracing结果 (临时中间贴图)
	 */
	UPROPERTY(VisibleDefaultsOnly)
	UTextureRenderTarget2D* PT_RT3;


	/**
	 *  主平行光
	 */
	UPROPERTY()
	ADirectionalLight* MainLight;

	/**
	 *  每个烘焙点的射线数量
	 */
	UPROPERTY(EditAnywhere)
	int SampleCount = 8;


	/**
	 *  每根射线弹射次数
	 */
	UPROPERTY(EditAnywhere)
	int Depth = 6;

	/**
	 *  path tracing距离衰减
	 */
	UPROPERTY(EditAnywhere)
	float MaxFallOff = 1.5f;

	/**
	 *  当前烘焙时间
	 */
	float Frame;

	/**
	 *  最大烘焙时间
	 */
	UPROPERTY(EditAnywhere)
	int MaxBakeTime = 30;

	/**
	 *  模糊次数
	 */
	UPROPERTY(EditAnywhere)
	int BlurTimes = 4;

	/**
	 *  模糊偏移强度
	 */
	UPROPERTY(EditAnywhere)
	float BlurIntensity = 2.4f;

	/**
	 *  高斯核 (暂时没有实际用途)
	 */
	static TArray<FSG_Full> OutSGs;

	/**
	 *  输出的SG光照贴图
	 */
	UPROPERTY(EditDefaultsOnly)
	TArray<UTextureRenderTarget2D*> SGRTs;

	/**
	 *  输出的SG光照贴图 (临时中间文件)
	 */
	UPROPERTY(EditDefaultsOnly)
	TArray<UTextureRenderTarget2D*> SGRTs_Temp;

	/**
	 *  输出的SG光照贴图 (临时中间文件)
	 */
	UPROPERTY(EditDefaultsOnly)
	TArray<UTextureRenderTarget2D*> SGRTs_Cur;

	/**
	 *  模型albedo贴图 (暂时只支持单个)
	 */
	UPROPERTY(EditAnywhere)
	UTexture2D* AlbedoTex;

	/**
	 *  模型材质粗糙度 (暂时只支持全局粗糙度)
	 */
	UPROPERTY(EditAnywhere)
	float Roughness;

	UPROPERTY(EditAnywhere)
	UMaterialParameterCollection *Collection;

	/**
	 * @brief 烘焙单帧光照贴图, 进行快速检验
	 */
	UFUNCTION(CallInEditor, Category = "Lightmap Baker | Debug")
	void FastBake();

	/**
	 * @brief 清除烘焙结果 (仅限RT)
	 */
	UFUNCTION(CallInEditor, Category = "Lightmap Baker | Debug")
	void ClearBakeMap();

	/**
	 * @brief 初始化高斯核
	 */
	void InitSGs();

	/**
	 * @brief 获得主光源
	 * @param _MainLight 
	 */
	ADirectionalLight* GetMainLight(ADirectionalLight* _MainLight);

	/**
	 * @brief 在烘焙点进行path tracing
	 * @param OutputRT 
	 * @param _Position_RT 
	 * @param _Normal_RT 
	 * @param _Tangent_RT 
	 * @param mainLight 
	 * @param sampleCount 
	 * @param depth 
	 * @param seed 
	 * @param OutSGRTs 
	 * @param _MaxFallOff 
	 * @param _Roughness 
	 */
	void PathTracingInLightmap(
		UTextureRenderTarget2D* OutputRT, UTextureRenderTarget2D* _Position_RT, UTextureRenderTarget2D* _Normal_RT, UTextureRenderTarget2D* _Tangent_RT,
		ADirectionalLight* mainLight, uint8 sampleCount, uint8 depth,
		float seed, 
		TArray<UTextureRenderTarget2D* >& OutSGRTs, float _MaxFallOff, float _Roughness);

	/**
	 * @brief 进行path tracing前的数据收集
	 * @param WorldContextObject 
	 * @param Output_Position_RT 
	 * @param Output_Normal_RT 
	 * @param Output_Tangent_RT 
	 */
	void UseRDGDraw(const UObject* WorldContextObject, UTextureRenderTarget2D* Output_Position_RT, UTextureRenderTarget2D* Output_Normal_RT, UTextureRenderTarget2D* Output_Tangent_RT);

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaTime) override;

	/**
	 * @brief 混合贴图
	 * @param PreTex 
	 * @param CurTex 
	 * @param OutTex 
	 */
	void BlendTexture(UTextureRenderTarget2D* PreTex, UTextureRenderTarget2D* CurTex, UTextureRenderTarget2D* OutTex);

	/**
	 * @brief 模糊贴图
	 * @param CurTex 
	 * @param TempTex 
	 * @param OutTex 
	 */
	void BlurTexture(UTextureRenderTarget2D* CurTex, UTextureRenderTarget2D* TempTex, UTextureRenderTarget2D* OutTex);

	/**
	 * @brief 按像素扩展贴图
	 * @param CurTex 
	 * @param TempTex 
	 * @param OutTex 
	 */
	void ExpandTexture(UTextureRenderTarget2D* CurTex, UTextureRenderTarget2D* TempTex, UTextureRenderTarget2D* OutTex);
};


/**
 * @brief 进行path tracing
 * @param RHIImmCmdList 
 * @param RenderTargetRHI 
 * @param Position_RT 
 * @param Normal_RT 
 * @param Tangent_RT 
 * @param _MainLight 
 * @param SampleCount 
 * @param depth 
 * @param seed 
 * @param OutSGs 
 * @param OutSGRTs 
 * @param InAlbedoTex 
 * @param MaxFallOff 
 * @param _Roughness 
 */
void ComputePathTracing(
	FRHICommandListImmediate &RHIImmCmdList,
	FTexture2DRHIRef RenderTargetRHI,
	FTexture2DRHIRef Position_RT,
	FTexture2DRHIRef Normal_RT,
	FTexture2DRHIRef Tangent_RT,
	ADirectionalLight* _MainLight,
	uint8 SampleCount,
	uint8 depth,
	float seed,
	TArray<FSG_Full>& OutSGs,
	TArray<FTexture2DRHIRef> OutSGRTs, FTexture2DRHIRef InAlbedoTex, float MaxFallOff, float _Roughness);
