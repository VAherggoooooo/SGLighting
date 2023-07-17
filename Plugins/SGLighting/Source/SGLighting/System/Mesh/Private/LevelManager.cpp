// Fill out your copyright notice in the Description page of Project Settings.


#include "SGLighting/System/Mesh/Public/LevelManager.h"

ULevelManager::ULevelManager()
{
	MeshCollecter = NewObject<UMeshCollecter>();
}

ULevelManager::~ULevelManager()
{
	MeshCollecter = nullptr;
}
