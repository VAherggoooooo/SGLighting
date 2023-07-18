
#include "SGLighting/System/Mesh/Public/BVHData.h"

UBVHData::UBVHData()
{
	
}

UBVHData::~UBVHData()
{
	this->MeshCollecter = nullptr;
}

void UBVHData::Init(UMeshCollecter* _meshCollecter)
{
	this->MeshCollecter = _meshCollecter;
}

int32 UBVHData::GetTrangleNum(bool bPrintNum, bool bPrintSM, bool bStatic) const
{
	if(MeshCollecter == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("No Mesh Collector"));
		return 0;
	}

	TArray<UStaticMesh*> staticMeshes = MeshCollecter->GetAllStaticMeshesInLevel(bPrintSM, bStatic);
	int32 trangleNum = 0;
	for (UStaticMesh* mesh : staticMeshes)
	{
		trangleNum = mesh->GetNumTriangles(mesh->GetMinLODIdx());
	}

	if(bPrintNum)
	{
		UE_LOG(LogTemp, Warning, TEXT("TrangleNum: %d"), trangleNum);
	}
	
	return trangleNum;
}

void UBVHData::GetTrangles()
{
	TArray<UStaticMesh*> staticMeshes = MeshCollecter->GetAllStaticMeshesInLevel();
	for (UStaticMesh* mesh : staticMeshes)
	{
		mesh->
	}
}
