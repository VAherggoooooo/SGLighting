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
    }//12个方向, 黄金分割算法 https://en.wikipedia.org/wiki/Golden_ratio
    
    for(int i = 0; i < N; ++i)
    {
        FSG_Full sample;
        sample.Axis = means[i].GetSafeNormal();//给SG的dir赋值
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

    for(int i = 0; i < numSGs; ++i)
        outSGs[i].Sharpness = sharpness;//赋值sharpness


    for(int i = 0; i < numSGs; ++i)
        outSGs[i].BasisSqIntegralOverDomain = 0.0f;
    
}
