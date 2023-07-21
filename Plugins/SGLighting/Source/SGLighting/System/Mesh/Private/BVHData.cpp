
#include "SGLighting/System/Mesh/Public/BVHData.h"
#include "KismetProceduralMeshLibrary.h"

UBVHData::UBVHData()
{

}

UBVHData::~UBVHData()
{
	this->MeshCollecter = nullptr;
	ClearSceneData();
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

void UBVHData::GetAllMeshInfo(TArray<FVector>& _Vertices, TArray<int32>& _Triangles, TArray<FVector>& _Normals, TArray<FVector2D>& _UVs, TArray<FProcMeshTangent>& _Tangents)
{
	TArray<UStaticMesh*> staticMeshes = MeshCollecter->GetAllStaticMeshesInLevel();
	// TArray<FVector> Vertices;
	// TArray<int32> Triangles;
	// TArray<FVector> Normals;
	// TArray<FVector2D> UVs;
	// TArray<FProcMeshTangent> Tangents;
	
	for (UStaticMesh* mesh : staticMeshes)
	{
		TArray<FVector> v;
		TArray<int32> tri;
		TArray<FVector> n;
		TArray<FVector2D> uv;
		TArray<FProcMeshTangent> tan;
		UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(mesh, mesh->GetMinLODIdx(), 0, v, tri, n, uv, tan);

		_Vertices.Append(v);
		_Triangles.Append(tri);
		_Normals.Append(n);
		_UVs.Append(uv);
		_Tangents.Append(tan);
	}
}

void UBVHData::GetSceneData()
{
	ClearSceneData();
	GetAllMeshInfo(Vertices, Triangles, Normals, UVs, Tangents);

	UE_LOG(LogTemp, Warning, TEXT("vNum: %d"), Vertices.Num());
	//UE_LOG(LogTemp, Warning, TEXT("triNum: %d"), Triangles.Num());
	//UE_LOG(LogTemp, Warning, TEXT("uv: %d"), UVs.Num());
}

void UBVHData::ClearSceneData()
{
	Vertices.Empty();
	Triangles.Empty();
	Normals.Empty();
	UVs.Empty();
	Tangents.Empty();
}

