
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
	if(_meshCollecter == nullptr)
	{
		_meshCollecter = NewObject<UMeshCollecter>();
	}
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

void UBVHData::GetAllMeshInfo(TArray<FVector>& _Vertices, TArray<FVector3f>& _Position, TArray<int32>& _VertexIDs, TArray<FVector>& _Normals, TArray<FVector2D>& _UVs, TArray<FVector2D>& _UVs2, TArray<FProcMeshTangent>& _Tangents, TArray<FMeshTriangle>& _Triangles)
{
	if(MeshCollecter == nullptr) return;
	TArray<UStaticMesh*> staticMeshes = MeshCollecter->GetAllStaticMeshesInLevel();
	
	// TArray<FVector> VerticeIDs;
	// TArray<int32> TriangleVertexIDs;
	// TArray<FVector> Normals;
	// TArray<FVector2D> UVs;
	// TArray<FProcMeshTangent> Tangents;
	
	for (UStaticMesh* mesh : staticMeshes)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s has %d Sections"), *(mesh->GetName()), mesh->GetNumSections(mesh->GetMinLODIdx()));
		
		for(int sectionID = 0; sectionID < mesh->GetNumSections(mesh->GetMinLODIdx()); sectionID++)
		{
			TArray<FVector> v;
			TArray<FVector3f> p;
			TArray<int32> tri;
			TArray<FVector> n;
			TArray<FVector2D> uv;
			TArray<FVector2D> uv2;
			TArray<FProcMeshTangent> tan;
			UKismetProceduralMeshLibrary::GetSectionFromStaticMesh(mesh, mesh->GetMinLODIdx(), sectionID, v, tri, n, uv, tan);

			//添加light uv
			const FStaticMeshLODResources& LOD = mesh->GetRenderData()->LODResources[mesh->GetMinLODIdx()];
			if (LOD.Sections.IsValidIndex(0))
			{
				for (int32 i = 0; i < v.Num(); i++)
				{
					FVector2f curUV = LOD.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(i, 1);
					uv2.Add(FVector2d(curUV.X,  curUV.Y));
					
					FVector3f curPos = LOD.VertexBuffers.PositionVertexBuffer.VertexPosition(i);
					FVector3f cp = FVector3f(curPos.X, curPos.Y, curPos.Z);
					p.Add(cp);
					//UE_LOG(LogTemp, Warning, TEXT("localPos: (%f, %f, %f)"), cp.X, cp.Y, cp.Z);
				}
			}
		
			_Vertices.Append(v);
			_VertexIDs.Append(tri);
			_Normals.Append(n);
			_UVs.Append(uv);
			_UVs2.Append(uv2);
			_Position.Append(p);
			_Tangents.Append(tan);

			TArray<FMeshTriangle> trian;
			for(int id = 0; id < _VertexIDs.Num(); id += 3)
			{
				FVector3f A =  _Position[_VertexIDs[id]];
				FVector3f B =  _Position[_VertexIDs[id + 1]];
				FVector3f C =  _Position[_VertexIDs[id + 2]];
				FVector3f NormalA = FVector3f(_Normals[_VertexIDs[id]]);
				FVector3f NormalB = FVector3f(_Normals[_VertexIDs[id + 1]]);
				FVector3f NormalC = FVector3f(_Normals[_VertexIDs[id + 2]]);
				trian.Add(FMeshTriangle(A,B,C, NormalA, NormalB, NormalC));
			}
			_Triangles.Append(trian);
			
		}
	}
}

void UBVHData::GetSceneData()
{
	ClearSceneData();
	GetAllMeshInfo(VerticeIDs,VerticePositions, TriangleVertexIDs, Normals, UVs, UVs2, Tangents, Triangles);

	//UE_LOG(LogTemp, Warning, TEXT("vNum: %d"), VerticeIDs.Num());
	//UE_LOG(LogTemp, Warning, TEXT("posNum: %d"), VerticePositions.Num());
	//UE_LOG(LogTemp, Warning, TEXT("triNum: %d"), TriangleVertexIDs.Num());
	
	//for (int32 i = 0; i < VerticeIDs.Num(); i++)
		//UE_LOG(LogTemp, Warning, TEXT("uv1: (%f, %f) : uv2: (%f, %f)"), UVs[i].X, UVs[i].Y, UVs2[i].X, UVs2[i].Y);

	// for (int32 i = 0; i < TriangleVertexIDs.Num(); i++)
	// 	UE_LOG(LogTemp, Warning, TEXT("triID: %d   pos: %s"), TriangleVertexIDs[i], *(VerticePositions[TriangleVertexIDs[i]].ToString()));
	//
	// for (int32 i = 0; i < Triangles.Num(); i++)
	// 	UE_LOG(LogTemp, Warning, TEXT("trianPos: %s,  %s,  %s"), *(Triangles[i].A.ToString()), *(Triangles[i].B.ToString()), *(Triangles[i].C.ToString()));
}

void UBVHData::ClearSceneData()
{
	VerticeIDs.Empty();
	VerticePositions.Empty();
	TriangleVertexIDs.Empty();
	Normals.Empty();
	UVs.Empty();
	UVs2.Empty();
	Tangents.Empty();
}

