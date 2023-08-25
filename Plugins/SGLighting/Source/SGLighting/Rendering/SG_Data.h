// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "SG_Data.generated.h"

/**
 * 
 */
UCLASS()
class SGLIGHTING_API USG_Data : public UObject
{
	GENERATED_BODY()
};

float DotProduct(FVector3f _v1, FVector3f _v2);

/*
 * SG lobe
 */
USTRUCT()
struct FSG_Full
{
	//exp(2 * Sharpness * (dot(Axis, Direction) - 1.0f)) integrated over the sampling domain.
	GENERATED_BODY()
public:
	FVector3f Axis;
	FVector3f Amplitude;
	float Sharpness;
	float BasisSqIntegralOverDomain;

	
	UPROPERTY(EditAnywhere)
	UTextureRenderTarget2D* OutputRT;

	FSG_Full()
	{
		Axis = FVector3f::Zero();
		Amplitude = FVector3f::Zero();
		Sharpness = 0;
		BasisSqIntegralOverDomain = 0;
		OutputRT = nullptr;
	}
	FSG_Full(FVector3f _Axis, FVector3f _Amplitude, float _sharpness, float _BasisSqIntegralOverDomain, UTextureRenderTarget2D* _OutputRT)
	{
		Axis = _Axis;
		Amplitude = _Amplitude;
		Sharpness = _sharpness;
		BasisSqIntegralOverDomain = _BasisSqIntegralOverDomain;
		OutputRT = _OutputRT;
	}

	FSG_Full(FVector3f _Axis, FVector3f _Amplitude, float _sharpness, UTextureRenderTarget2D* _OutputRT)
	{
		Axis = _Axis;
		Amplitude = _Amplitude;
		Sharpness = _sharpness;
		BasisSqIntegralOverDomain = 0.0f;
		OutputRT = _OutputRT;
	}

	FSG_Full(FVector3f _Axis, FVector3f _Amplitude, UTextureRenderTarget2D* _OutputRT)
	{
		Axis = _Axis;
		Amplitude = _Amplitude;
		Sharpness = 1.0f;
		BasisSqIntegralOverDomain = 0.0f;
		OutputRT = _OutputRT;
	}

	
                           
	FVector3f EvaluateSG(FVector3f dir)
	{
		return Amplitude * exp(Sharpness * (DotProduct(dir, Axis) - 1.0f));
	}
};

struct FSG
{
	FVector3f Axis;
	FVector3f Amplitude;
	float Sharpness;
	float BasisSqIntegralOverDomain;

	FSG(){Axis = FVector3f(); Amplitude = FVector3f(); Sharpness = 0; BasisSqIntegralOverDomain = 0;}
	FSG(FVector3f _Axis, FVector3f _Amplitude, float _Sharpness, float _BasisSqIntegralOverDomain)
	{
		Axis = _Axis;
		Amplitude = _Amplitude;
		Sharpness = _Sharpness;
		BasisSqIntegralOverDomain = _BasisSqIntegralOverDomain;
	}
};



float DotProduct(FVector3f _v1, FVector3f _v2)
{
	return _v1.X * _v2.X + _v1.Y * _v2.Y + _v1.Z * _v2.Z;
}


void GenerateUniformSGs(int numSGs, TArray<FSG_Full>& outSGs);



/**
 * @brief SG核数量, 拉满12, 可以是5, 9
 */
#define SG_NUM 12


struct FTextureVertex
{
	FVector4f Position;
	FVector2f UV;
	FVector4f Position_OS;
	FVector4f Normal;
	FVector4f Tangent;
};
