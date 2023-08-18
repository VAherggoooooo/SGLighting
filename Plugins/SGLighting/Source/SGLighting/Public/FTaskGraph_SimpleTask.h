#pragma once
#include "CoreMinimal.h"
#include "Engine/DirectionalLight.h"
#include "HAL/ThreadManager.h"
#include "SGLighting/Rendering/MakeLightmapRDG.h"
#include "SGLighting/Rendering/SGPathTracing.h"

class FTaskGraph_SimpleTask
{
	FString m_ThreadName;
	UTextureRenderTarget2D* OutputRT;
	UTextureRenderTarget2D* Position_RT;
	UTextureRenderTarget2D* Normal_RT;
	UTextureRenderTarget2D* Tangent_RT;
	ADirectionalLight* MainLight;
	uint8 SampleCount;
	uint8 depth;
	float seed;
public:
	FTaskGraph_SimpleTask(
		const FString& ThreadName,
		UTextureRenderTarget2D* OutputRT,
		UTextureRenderTarget2D* Position_RT,
		UTextureRenderTarget2D* Normal_RT,
		UTextureRenderTarget2D* Tangent_RT,
		ADirectionalLight* MainLight,
		uint8 SampleCount = 200,
		uint8 depth = 6,
		float seed = 0) :
	m_ThreadName(ThreadName),
	OutputRT(OutputRT),
	Position_RT(Position_RT),
	Normal_RT(Normal_RT),
	Tangent_RT(Tangent_RT),
	MainLight(MainLight),
	SampleCount(SampleCount),
	depth(depth),
	seed(seed)
	{}

	
	~FTaskGraph_SimpleTask(){}
 
	// 固定写法
	FORCEINLINE TStatId GetStatId() const
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FTaskGraph_SimpleTask, STATGROUP_TaskGraphTasks);
	}
 
	// 指定在哪个线程运行
	static ENamedThreads::Type GetDesiredThread() { return ENamedThreads::GameThread; }//AnyThread
  
	// 后续执行模式
	static ESubsequentsMode::Type GetSubsequentsMode() { return ESubsequentsMode::TrackSubsequents; }
 
	// 线程逻辑执行函数
	void DoTask(ENamedThreads::Type CurrentThread, const FGraphEventRef& MyCompletionGraphEvent)
	{
		UE_LOG(LogTemp, Warning, TEXT("Thread %s Begin!"), *m_ThreadName);
		//USGPathTracing::PathTracingInLightmap(nullptr, OutputRT, Position_RT, Normal_RT, Tangent_RT, MainLight, SampleCount, depth, seed);
		UE_LOG(LogTemp, Warning, TEXT("Thread %s End!"), *m_ThreadName);
	}
};
