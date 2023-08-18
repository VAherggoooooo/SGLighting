// Fill out your copyright notice in the Description page of Project Settings.


#include "SG_Data.h"

#include "MathUtil.h"


void GenerateUniformSGs(int numSGs, TArray<FSG_Full>& outSGs)
{
	const int N = numSGs;
	TArray<FVector3f> means;
    means.SetNum(N);
    //outSGs.SetNum(N);

	float inc = PI * (3.0f - sqrt(5.0f));
    float off = 2.0f / N;
    for(int k = 0; k < N; ++k)
    {
        float y = k * off - 1.0f + (off / 2.0f);
        float r = sqrt(1.0f - y * y);
        float phi = k * inc;
        means[k] = FVector3f(FMathf::Cos(phi) * r, FMathf::Sin(phi) * r, y);
    }//随机了12个方向, 某种算法
    
    for(int i = 0; i < N; ++i)
    {
        FSG_Full sample;
        sample.Axis = means[i].GetSafeNormal();//仅仅给SG的dir赋值
        outSGs[i] = sample;//添加到输出的SG
    }

    //计算sharpness
    float minDP = 1.0f;
    for(int i = 1; i < numSGs; ++i)
    {
        FVector3f h = outSGs[i].Axis + outSGs[0].Axis;//0号SG与当前SG轴向的半角向量
        h = h.GetSafeNormal();
        minDP =  FMathf::Min(minDP, DotProduct(h, outSGs[0].Axis));//找最小
    }
    float mdp = (minDP - 1.0f);
    float sharpness = (log(0.65f) * numSGs) / ((minDP - 1.0f) + 0.000001f);
    //UE_LOG(LogTemp, Warning, TEXT("sharpness %f"), sharpness);
    for(int i = 0; i < numSGs; ++i)
        outSGs[i].Sharpness = sharpness;//赋值sharpness


    for(int i = 0; i < numSGs; ++i)
        outSGs[i].BasisSqIntegralOverDomain = 0.0f;

    
    //const uint64 sampleCount = 2048;
    //FVector2f samples[sampleCount];
    // //计算BasisSqIntegralOverDomain
    // for(uint64 i = 0; i < sampleCount; ++i)
    // {
    //     //SampleDirectionSphere(samples[i].x, samples[i].y)
    //     Float3 dir = distribution == SGDistribution::Hemispherical ? SampleDirectionHemisphere(samples[i].x, samples[i].y)
    //                                                                : SampleDirectionSphere(samples[i].x, samples[i].y);
    //     for(uint32 j = 0; j < numSGs; ++j)
    //     {
    //         float weight = std::exp(outSGs[j].Sharpness * (Float3::Dot(dir, outSGs[j].Axis) - 1.0f));
    //         outSGs[j].BasisSqIntegralOverDomain += (weight * weight - outSGs[j].BasisSqIntegralOverDomain) / float(i + 1);
    //     }
    // }
}
