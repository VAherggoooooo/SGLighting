// Fill out your copyright notice in the Description page of Project Settings.


#include "SGLighting/System/Mesh/Public/LevelManager.h"

ULevelManager::ULevelManager()
{
	this->MeshCollecter = NewObject<UMeshCollecter>();
	this->BVHData = NewObject<UBVHData>();
	this->LightmapCollect = NewObject<ULightmapCollect>();

	this->MeshCollecter->Init();
	this->BVHData->Init(this->MeshCollecter);
	this->LightmapCollect->Init(this->BVHData);
}

ULevelManager::~ULevelManager()
{
	this->MeshCollecter = nullptr;
	this->BVHData = nullptr;
	this->LightmapCollect = nullptr;
}
