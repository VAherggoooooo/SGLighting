#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "SGLighting/Actor/Public/LightmapBaker.h"
#include "BakeVisualMgr.generated.h"

UCLASS()
class SGLIGHTING_API ABakeVisualMgr : public AActor
{
	GENERATED_BODY()

public:

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
	
	

	UPROPERTY(VisibleDefaultsOnly)
	USceneComponent* Root;
	
	UPROPERTY(EditDefaultsOnly)
	UInstancedStaticMeshComponent* StaticMeskComp;
	
	TArray<FVector> TransformList;

	ALightmapBaker* LightmapBaker;

	ABakeVisualMgr();

	UFUNCTION(CallInEditor)
	void ShowBakePoints();
	
protected:
	virtual void BeginPlay() override;

public:
	virtual void Tick(float DeltaTime) override;
};
