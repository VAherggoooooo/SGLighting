// Copyright Epic Games, Inc. All Rights Reserved.

#include "MyCustomShader.h"

#include "Interfaces/IPluginManager.h"

#define LOCTEXT_NAMESPACE "FMyCustomShaderModule"

void FMyCustomShaderModule::StartupModule()
{
	//shader文件虚拟路径映射
	FString PluginShaderDir = FPaths::Combine(IPluginManager::Get().FindPlugin(TEXT("MyCustomShader"))->GetBaseDir(), TEXT("Shaders"));
	AddShaderSourceDirectoryMapping(TEXT("/Plugins/MyCustomShader"), PluginShaderDir);
}

void FMyCustomShaderModule::ShutdownModule()
{
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FMyCustomShaderModule, MyCustomShader)